#include "ui/GridRenderer.hpp"
#include "materials/MaterialRegistry.hpp"
#include "ui/AddWallCommand.hpp"
#include "ui/DeleteWallCommand.hpp"
#include "ui/RemoveEntityCommand.hpp"
#include "ui/AddEntityCommand.hpp"
#include "ui/AddOpeningCommand.hpp"
#include "ui/RemoveOpeningCommand.hpp"
#include "ui/ShiftGeometryCommand.hpp"
#include "ui/AddWallLayerCommand.hpp"
#include "ui/RemoveWallLayerCommand.hpp"
#include "ui/EditWallLayerCommand.hpp"
#include "ui/UiLog.hpp"
#include "ui/Cosmetics.hpp"
#include "calc/SceneCompiler.hpp"
#include "calc/CompilerOutput.hpp"
#include "output/PrintCompilerOutput.hpp"
#include "output/PrintCompilerOutputUI.hpp"
#include <imgui.h>
#include <imgui-SFML.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>


extern MaterialRegistry material_registry;

namespace {

    constexpr double SELECT_EPS_CM = 25.0;          // select wall/point by clicking within 25cm of it
    constexpr double SNAP_POINT_EPS_CM = 0.01;      // snap epsilon, checking if p equals an existing endpoint for selection logic
    constexpr float PICK_RADIUS_PX = 10.0f;
    constexpr double SHIFT_MOVE_MULTIPLIER = 10.0;  // when shift is pressed, the drawing shifts SHIFT_MOVE_MULTIPLIER times the normal distance
    constexpr float TOOLBAR_HEIGHT_PX = 40.f;       // toolbar
    constexpr float RIGHT_PANEL_WIDTH_PX = 300.f;   // terminal
    constexpr float LEFT_PANEL_WIDTH_PX = 300.f;    // user input

    double distance_point_to_segment(Point p, Point a, Point b) {
        const double dx = b.x_cm - a.x_cm;
        const double dy = b.y_cm - a.y_cm;

        if (dx == 0.0 && dy == 0.0) {
            return std::hypot(p.x_cm - a.x_cm, p.y_cm - a.y_cm);
        }

        const double t = ((p.x_cm - a.x_cm) * dx + (p.y_cm - a.y_cm) * dy) / (dx * dx + dy * dy);
        const double clamped = std::clamp(t, 0.0, 1.0);
        const double proj_x = a.x_cm + clamped * dx;
        const double proj_y = a.y_cm + clamped * dy;

        return std::hypot(p.x_cm - proj_x, p.y_cm - proj_y);
    }

    double distance_point_to_point(Point a, Point b) {
        return std::hypot(a.x_cm - b.x_cm, a.y_cm - b.y_cm);
    }

    bool snaps_to_existing_point(const Point& p, const GeometryEngine& engine) {

        for (const auto& w : engine.get_walls()) {

            if (std::hypot(p.x_cm - w.a.x_cm, p.y_cm - w.a.y_cm) < SNAP_POINT_EPS_CM) {
                return true;
            }

            if (std::hypot(p.x_cm - w.b.x_cm, p.y_cm - w.b.y_cm) < SNAP_POINT_EPS_CM) {
                return true;
            }
        }
        return false;
    }

    static double project_t_onto_wall(const Point& p, const Wall& w) {
        double dx = w.b.x_cm - w.a.x_cm;
        double dy = w.b.y_cm - w.a.y_cm;
        double len2 = dx * dx + dy * dy;
        if (len2 < 1e-9) {
            return 0.0;
        }
        double t = ((p.x_cm - w.a.x_cm) * dx + (p.y_cm- w.a.y_cm) * dy) / len2;

        return std::clamp(t, 0.0, 1.0);
    }

    static Point lerp_point(const Point& a, const Point& b, double t) {
        return {a.x_cm + t * (b.x_cm - a.x_cm), a.y_cm + t * (b.y_cm - a.y_cm)};
    }

    sf::RectangleShape make_thick_segment(sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color color) {

        sf::Vector2f d = b - a;
        float length = std::sqrt(d.x * d.x + d.y * d.y);
        sf::RectangleShape r({length, thickness});
        r.setFillColor(color);
        r.setOrigin(sf::Vector2f{0.f, thickness * 0.5f});
        r.setPosition(a);
        r.setRotation(sf::degrees(std::atan2(d.y, d.x) * 180.f / M_PI));
        
        return r;
    }


    static bool ToolbarButton(const char* label, bool active) {
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        bool clicked = ImGui::Button(label);

        if (active) {
            ImGui::PopStyleColor(3);
        }

        return clicked;
    }

} // end of anonymous namespace


GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e, AppState& state) : window(w), engine(e), app_state(state),length_text(font) {
    
    window_size = window.getSize();
    const auto& bounds = engine.get_world_bounds();
    grid_view.setSize(sf::Vector2f{static_cast<float>(bounds.width_cm), static_cast<float>(bounds.height_cm)});
    grid_view.setCenter(sf::Vector2f{static_cast<float>(bounds.width_cm * 0.5), static_cast<float>(bounds.height_cm * 0.5)});
    float win_w = static_cast<float>(window_size.x);
    float win_h = static_cast<float>(window_size.y);
    float world_w = static_cast<float>(bounds.width_cm);
    float world_h = static_cast<float>(bounds.height_cm);
    float window_aspect = win_w / win_h;
    float world_aspect = world_w / world_h;

    sf::FloatRect viewport;
    if (window_aspect > world_aspect) { // window is wider than world -> pillarbox
        float width = world_aspect / window_aspect;
        viewport = {{(1.f - width) / 2.f, 0.f}, {width, 1.f}};
    } else { // window is taller than world -> letterbox
        float height = window_aspect / world_aspect;
        viewport = {{0.f, (1.f - height) / 2.f}, {1.f, height}};
    }
    update_viewport();
    if (!font.openFromFile("assets/fonts/Inter-Regular.ttf")) {
        std::cerr << "Failed to load font\n" << std::endl;
    }
    length_text.setFont(font);
    length_text.setCharacterSize(18);
    length_text.setFillColor(sf::Color::Black);
}


double GridRenderer::pixel_radius_to_world_cm(float px) const {
    sf::Vector2f p0 = window.mapPixelToCoords({0, 0}, grid_view);
    sf::Vector2f p1 = window.mapPixelToCoords({(int)px, 0}, grid_view);
    
    return std::abs(p1.x - p0.x);
}

void GridRenderer::update_viewport() {
    const float win_w = static_cast<float>(window.getSize().x);
    const float win_h = static_cast<float>(window.getSize().y);

    if (win_w <= 0.f || win_h <= 0.f) { return; }

    const float top_norm = TOOLBAR_HEIGHT_PX / win_h;
    const float side_norm = RIGHT_PANEL_WIDTH_PX / win_w;

    sf::FloatRect viewport;
    viewport.position.x = side_norm;
    viewport.position.y = top_norm;
    viewport.size.x = 1.f - 2.f * side_norm;
    viewport.size.y = 1.f - top_norm;

    grid_view.setViewport(viewport);
}


bool GridRenderer::load_background_image(const std::string& path) {
    if (!background_texture.loadFromFile(path)) {
        std::cerr << "Failed to load background image: " << path << "\n";
        return false;
    }

    const auto size_px = background_texture.getSize();
    const auto& bounds = engine.get_world_bounds();
    engine.set_world_bounds_from_image(size_px.x, size_px.y);
    grid_view.setSize(sf::Vector2f{static_cast<float>(bounds.width_cm), static_cast<float>(bounds.height_cm)});
    grid_view.setCenter(sf::Vector2f{static_cast<float>(bounds.width_cm * 0.5f), static_cast<float>(bounds.height_cm * 0.5f)});
    update_viewport();
    background_sprite = std::make_unique<sf::Sprite>(background_texture);
    background_sprite->setPosition(sf::Vector2f{0.f, 0.f});
    std::cout << "Background loaded: " << size_px.x << " x " << size_px.y << " px\n";

    return true;
}


void GridRenderer::finalize_blueprint() {

    if (!blueprint_finalized) {
        auto errors = engine.validate();
        if (!errors.empty()) {
            ui_log.push("VALIDATION ERRORS: ");
            for (const auto& e : errors) {
                ui_log.push("  - " + e.message);
            }
            return;
        }
        ui_log.push("Validation passed.");
    }

    blueprint_finalized = !blueprint_finalized;

    drawing = false;
    placing_opening = false;
    selection.clear();

    if (blueprint_finalized) {

        ui_log.clear();
        ui_log.push("========= Final Blueprint =========");

        // canonical JSON
        nlohmann::json j = engine.to_json();
        ui_log.push(j.dump(2));

        // compile
        calc::CalcScene scene = calc::SceneCompiler::compile(j);

        ui_log.separator();
        ui_log.push("Compiled scene counts:");
        ui_log.push("  sources: " + std::to_string(scene.sources.size()));
        ui_log.push("  dose_points: " + std::to_string(scene.dose_points.size()));
        ui_log.push("  walls: " + std::to_string(scene.walls.size()));

        calc::CompilerOutput compiler_output = calc::build_compiler_output(scene);
        ui_log.separator();
        output::print_to_ui(compiler_output, ui_log);
        // uncomment for terminal debugging
        output::print(compiler_output);

    } else {
        ui_log.clear();
        ui_log.push("========= Editing Resumed =========");
    }
}


void GridRenderer::handle_select_click(const Point& p) {
    selection.clear();

    // entity selection
    {
        const auto& entities = engine.get_entities();
        double best_entity_dist = SELECT_EPS_CM;

        for (std::size_t i = 0; i < entities.size(); ++i) {
            double d = distance_point_to_point(p, entities[i].position);

            if (d < best_entity_dist) {
                best_entity_dist = d;
                selection.type = Selection::Type::Entity;
                selection.entity_index = i;

                if (entities[i].type == PointType::Source) {
                    inspector_source_index = i;
                } else if (entities[i].type == PointType::Dose) {
                    inspector_dose_index = i;
                }
            }
        }

        if (selection.type == Selection::Type::Entity) { return; }
    }

    // opening selection
    {
        const auto& walls = engine.get_walls();
        double best_dist = SELECT_EPS_CM;

        for (std::size_t wi = 0; wi < walls.size(); ++wi) {
            const auto& w = walls[wi];

            for (std::size_t oi = 0; oi < w.openings.size(); ++oi) {
                const auto& o = w.openings[oi];

                double half_t = (o.length_cm * 0.5) / w.length_cm;
                double t0 = std::clamp(o.center_t - half_t, 0.0, 1.0);
                double t1 = std::clamp(o.center_t + half_t, 0.0, 1.0);

                Point p0 = lerp_point(w.a, w.b, t0);
                Point p1 = lerp_point(w.a, w.b, t1);

                double d = distance_point_to_segment(p, p0, p1);
                if (d < best_dist) {
                    best_dist = d;
                    selection.type = Selection::Type::Opening;
                    selection.wall_index = wi;
                    selection.opening_index = oi;
                }
            }
        }

        if (selection.type == Selection::Type::Opening) { return; }
    }

    // wall selection
    {
        const auto& walls = engine.get_walls();
        double best_wall_dist = SELECT_EPS_CM;

        for (std::size_t i = 0; i < walls.size(); ++i) {
            double d = distance_point_to_segment(p, walls[i].a, walls[i].b);

            if (d < best_wall_dist) {
                best_wall_dist = d;
                selection.type = Selection::Type::Wall;
                selection.wall_index = i;
                inspector_wall_index = i;
            }
        }
    }
}


double GridRenderer::distance_cm(Point a, Point b) const {
    // euclidean distance
    const double dx = b.x_cm - a.x_cm;
    const double dy = b.y_cm - a.y_cm;

    return std::sqrt(dx * dx + dy * dy);
}


void GridRenderer::draw_project_picker() {
    ImGui::SetNextWindowSize(ImVec2(420, 220), ImGuiCond_Always);
    ImGui::SetNextWindowPos(
        ImVec2(window.getSize().x * 0.5f, window.getSize().y * 0.5f),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f)
    );

    ImGui::Begin(
        "Project Picker",
        nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );

    ImGui::Text("XRCT Shielding");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("New Project", ImVec2(180, 40))) {
        app_state.mode = AppMode::NewProjectSetup;
        scale_has_p1 = false;
        scale_has_p2 = false;
        update_viewport();
    }

    ImGui::Spacing();
    ImGui::BeginDisabled();
    ImGui::Button("Open Project", ImVec2(180, 40));
    ImGui::EndDisabled();
    ImGui::End();
}


void GridRenderer::draw_new_project_setup() {
    ImGui::SetNextWindowPos(ImVec2(10, TOOLBAR_HEIGHT_PX + 10));
    ImGui::SetNextWindowSize(ImVec2(LEFT_PANEL_WIDTH_PX - 20, 220));

    ImGui::Begin("Project Setup Mode", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse
    );

    ImGui::Text("Scale Calibration");
    ImGui::Separator();

    ImGui::TextWrapped(
        "Click two points on the grid that represent a known real-world distance."
    );

    ImGui::Spacing();

    ImGui::Text("Real-world distance (cm)");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputDouble("##scale_cm", &scale_real_distance_cm, 10.0, 100.0);


    if (scale_has_p1 && scale_has_p2) {
        double pixel_dist = distance_cm(scale_p1, scale_p2);
        ImGui::TextWrapped("Measured distance on plan: %.2f grid units", pixel_dist);
        ImGui::TextWrapped(
            "Grid units are arbitrary until calibrated. "
            "They will be converted to centimeters using the real-world distance you enter. "
            "Press 'Apply Scale' to calibrate and proceed to editing. "
            "If you would like to re-calibrate the scale once entering editing mode, press the 'Edit Scale' button to return to this page. "
        );
        if (ImGui::Button("Apply Scale")) {
            double measured_draw_distance = distance_cm(scale_p1, scale_p2);
            if (measured_draw_distance > 0.0) {
                engine.set_distance_scale(scale_real_distance_cm / measured_draw_distance);
            }

            update_viewport();
            app_state.mode = AppMode::Editing;
        }
    } else {
        ImGui::TextDisabled("Select two points on the grid.");
    }

    ImGui::Spacing();

    if (ImGui::Button("Reset Points")) {
        scale_has_p1 = false;
        scale_has_p2 = false;
    }

    ImGui::End();
}


void GridRenderer::draw_toolbar() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(window.getSize().x, TOOLBAR_HEIGHT_PX));

    ImGui::Begin("TopToolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    if (ToolbarButton("SELECT", interaction_mode == InteractionMode::Select)) {
        interaction_mode = InteractionMode::Select;
        current_tool = Tool::None;
        selection.clear();
        drawing = false;
        placing_opening = false;
    }
    ImGui::SameLine();

    if (ToolbarButton("DRAW", interaction_mode == InteractionMode::Draw)) {
        interaction_mode = InteractionMode::Draw;
        selection.clear();
        drawing = false;
        placing_opening = false;
    }
    ImGui::SameLine();

    ImGui::Separator();
    ImGui::SameLine();

    if (ToolbarButton("Wall", current_tool == Tool::DrawWall)) {
        interaction_mode = InteractionMode::Draw;
        current_tool = Tool::DrawWall;
        drawing = false;
    }
    ImGui::SameLine();

    if (ToolbarButton("Source", current_tool == Tool::PlaceSource)) {
        interaction_mode = InteractionMode::Draw;
        current_tool = Tool::PlaceSource;
    }
    ImGui::SameLine();

    if (ToolbarButton("Dose", current_tool == Tool::PlaceDose)) {
        interaction_mode = InteractionMode::Draw;
        current_tool = Tool::PlaceDose;
    }
    ImGui::SameLine();

    if (ToolbarButton("Opening", current_tool == Tool::PlaceOpen)) {
        interaction_mode = InteractionMode::Draw;
        current_tool = Tool::PlaceOpen;
        placing_opening = false;
    }
    ImGui::SameLine();

    if (ToolbarButton("Window", current_tool == Tool::PlaceWindow)) {
        interaction_mode = InteractionMode::Draw;
        current_tool = Tool::PlaceWindow;
        placing_opening = false;
    }
    ImGui::SameLine();

    if (ToolbarButton("Door", current_tool == Tool::PlaceDoor)) {
        interaction_mode = InteractionMode::Draw;
        current_tool = Tool::PlaceDoor;
        placing_opening = false;
    }
    ImGui::SameLine();

    ImGui::Separator();
    ImGui::SameLine();

    if (ToolbarButton("Undo", false)) {
        undo_stack.undo();
    }
    ImGui::SameLine();

    if (ToolbarButton("Redo", false)) {
        undo_stack.redo();
    }
    ImGui::SameLine();

    if (ToolbarButton("Remove", false)) {
        switch (selection.type) {

            case Selection::Type::Wall:
                undo_stack.execute(std::make_unique<DeleteWallCommand>(engine, selection.wall_index));
                selection.clear();
                break;

        case Selection::Type::Opening:
                undo_stack.execute(
                    std::make_unique<RemoveOpeningCommand>(engine, selection.wall_index, selection.opening_index));
                selection.clear();
                break;

        case Selection::Type::Entity:
                undo_stack.execute(std::make_unique<RemoveEntityCommand>(engine, selection.entity_index));
                selection.clear();
                break;

            default:
                break;
        }
    }
    ImGui::SameLine();

    ImGui::Separator();
    ImGui::SameLine();

    if (ToolbarButton("Finalize", false)) {
        finalize_blueprint();
    }
    ImGui::SameLine();

    if (ToolbarButton("Edit Scale", false)) {
        app_state.mode = AppMode::NewProjectSetup;
        scale_has_p1 = false;
        scale_has_p2 = false;
    }

    ImGui::End();
}


void GridRenderer::draw_left_panel() {
    ImGui::SetNextWindowPos(ImVec2(0.f, TOOLBAR_HEIGHT_PX));
    ImGui::SetNextWindowSize(
        ImVec2(LEFT_PANEL_WIDTH_PX, window.getSize().y - TOOLBAR_HEIGHT_PX)
    );

    ImGui::Begin(
        "LeftPanel",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse
    );

    if (ImGui::BeginTabBar("LeftPanelTabs")) {

        draw_wall_tab();
        draw_source_tab();
        draw_dose_tab();

        ImGui::EndTabBar();
    }

    ImGui::End();
}


void GridRenderer::draw_wall_tab() {
    if (!ImGui::BeginTabItem("Wall"))
        return;

    auto& walls = engine.get_walls_mutable();

    if (walls.empty()) {
        ImGui::TextDisabled("No walls created yet.");
        ImGui::EndTabItem();
        return;
    }

    if (selection.type == Selection::Type::Wall || selection.type == Selection::Type::WallLayer) {
        inspector_wall_index = selection.wall_index;
    }

    // Default inspector target
    if (!inspector_wall_index.has_value())
        inspector_wall_index = 0;

    // Wall selector
    std::vector<std::string> wall_labels;
    for (size_t i = 0; i < walls.size(); ++i)
        wall_labels.push_back("Wall " + std::to_string(i + 1));

    static int wall_idx = 0;
    wall_idx = static_cast<int>(*inspector_wall_index);

    if (ImGui::Combo(
            "Wall",
            &wall_idx,
            [](void* data, int idx, const char** out) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                *out = labels[idx].c_str();
                return true;
            },
            &wall_labels,
            wall_labels.size()))
    {
        inspector_wall_index = wall_idx;
        selection.clear();
        selection.type = Selection::Type::Wall;
        selection.wall_index = wall_idx;
    }

    auto& wall = walls[*inspector_wall_index];

    ImGui::Separator();
    ImGui::Text("Length: %.1f cm", wall.length_cm * engine.get_distance_scale());

    if (ImGui::BeginTabBar("WallLayers")) {

        for (size_t i = 0; i < wall.layers.size(); ++i) {
            std::string label = "Layer " + std::to_string(i + 1);

            if (ImGui::BeginTabItem(label.c_str())) {
                auto& layer = wall.layers[i];

                // Material
                int mat_id = layer.material_id;
                if (ImGui::InputInt("Material ID", &mat_id)) {
                    layer.material_id = mat_id;
                }

                // Thickness
                ImGui::InputDouble("Thickness (cm)", &layer.thickness_cm);

                // Remove layer
                if (wall.layers.size() > 1) {
                    if (ImGui::Button("Remove Layer")) {
                        undo_stack.execute(
                            std::make_unique<RemoveWallLayerCommand>(
                                engine, *inspector_wall_index, i));
                        ImGui::EndTabItem();
                        break;
                    }
                }

                ImGui::EndTabItem();
            }
        }

        // Add layer button
        if (ImGui::TabItemButton("+")) {
            WallLayer new_layer;
            new_layer.material_id = wall.layers.back().material_id;
            new_layer.thickness_cm = 10.0;

            undo_stack.execute(
                std::make_unique<AddWallLayerCommand>(
                    engine, *inspector_wall_index,
                    wall.layers.size() - 1,
                    new_layer));
        }

        ImGui::EndTabBar();
    }

    ImGui::EndTabItem();
}


void GridRenderer::draw_source_tab() {
    if (!ImGui::BeginTabItem("Source"))
        return;

    auto& entities = engine.get_entities_mutable();

    std::vector<size_t> sources;
    for (size_t i = 0; i < entities.size(); ++i)
        if (entities[i].type == PointType::Source)
            sources.push_back(i);

    if (sources.empty()) {
        ImGui::TextDisabled("No source points created yet.");
        ImGui::EndTabItem();
        return;
    }

    if (selection.type == Selection::Type::Entity) {
        const auto& e = engine.get_entities()[selection.entity_index];
        if (e.type == PointType::Source) {
            inspector_source_index = selection.entity_index;
        }
    }

    if (!inspector_source_index.has_value())
        inspector_source_index = sources[0];

    // Source selector
    std::vector<std::string> labels;
    for (size_t i = 0; i < sources.size(); ++i)
        labels.push_back("Source " + std::to_string(i + 1));

    static int src_idx = 0;
    src_idx = std::find(sources.begin(), sources.end(), *inspector_source_index) - sources.begin();

    if (ImGui::Combo(
            "Source",
            &src_idx,
            [](void* data, int idx, const char** out) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                *out = labels[idx].c_str();
                return true;
            },
            &labels,
            labels.size()))
    {
        inspector_source_index = sources[src_idx];
        selection.clear();
        selection.type = Selection::Type::Entity;
        selection.entity_index = inspector_source_index.value();
    }

    auto& e = entities[*inspector_source_index];
    auto& s = *e.source;

    ImGui::Separator();
    char name_buf[64];
    std::snprintf(name_buf, sizeof(name_buf), "%s", e.label.c_str());
    if (ImGui::InputText("Name", name_buf, sizeof(name_buf))) {
        e.label = name_buf;
    }

    ImGui::InputFloat("Patients / week", &s.num_patients);
    ImGui::InputFloat("Activity / patient (MBq)", &s.activity_per_patient_MBq);
    ImGui::InputFloat("Uptake (hours)", &s.uptake_time_hours);
    ImGui::Checkbox("Patient attenuation", &s.apply_patient_attenuation);
    ImGui::Checkbox("Radioactive decay", &s.apply_radioactive_decay);

    ImGui::EndTabItem();
}


void GridRenderer::draw_dose_tab() {
    if (!ImGui::BeginTabItem("Dose"))
        return;

    auto& entities = engine.get_entities_mutable();

    std::vector<size_t> doses;
    for (size_t i = 0; i < entities.size(); ++i)
        if (entities[i].type == PointType::Dose)
            doses.push_back(i);

    if (doses.empty()) {
        ImGui::TextDisabled("No dose points created yet.");
        ImGui::EndTabItem();
        return;
    }

    if (selection.type == Selection::Type::Entity) {
        const auto& e  = engine.get_entities()[selection.entity_index];
        if (e.type == PointType::Dose) {
            inspector_dose_index = selection.entity_index;
        }
    }

    if (!inspector_dose_index.has_value())
        inspector_dose_index = doses[0];

    std::vector<std::string> labels;
    for (size_t i = 0; i < doses.size(); ++i)
        labels.push_back("Dose " + std::to_string(i + 1));

    static int dose_idx = 0;
    dose_idx = std::find(doses.begin(), doses.end(), *inspector_dose_index) - doses.begin();

    if (ImGui::Combo(
            "Dose",
            &dose_idx,
            [](void* data, int idx, const char** out) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                *out = labels[idx].c_str();
                return true;
            },
            &labels,
            labels.size()))
    {
        inspector_dose_index = doses[dose_idx];
        selection.clear();
        selection.type = Selection::Type::Entity;
        selection.entity_index = inspector_dose_index.value();
    }

    auto& e = entities[*inspector_dose_index];
    auto& d = *e.dose;

    ImGui::Separator();
    char name_buf[64];
    std::snprintf(name_buf, sizeof(name_buf), "%s", e.label.c_str());
    if (ImGui::InputText("Name", name_buf, sizeof(name_buf))) {
        e.label = name_buf;
    }

    ImGui::InputFloat("Occupancy", &d.occupancy);
    ImGui::InputFloat("Dose limit (uSv)", &d.dose_limit_uSv);

    ImGui::EndTabItem();
}


void GridRenderer::handle_events() {
    while (const auto event = window.pollEvent()) {

        ImGui::SFML::ProcessEvent(window, *event);

        if (app_state.mode == AppMode::ProjectPicker) {
            continue;
        }

        const ImGuiIO& io = ImGui::GetIO();
        
        // close
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        // resize
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            window_size = {resized->size.x, resized->size.y};
            update_viewport();
        }

        // zoom
        if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {

            if (io.WantCaptureMouse) { continue; }
            if (wheel->wheel != sf::Mouse::Wheel::Vertical) { continue; }

            const float zoom_factor = (wheel->delta > 0) ? 0.9f : 1.1f;
            zoom *= zoom_factor;
            zoom = std::clamp(zoom, 0.1f, 5.0f);
            sf::Vector2i mouse_px = sf::Mouse::getPosition(window);
            sf::Vector2f before = window.mapPixelToCoords(mouse_px, grid_view);
            grid_view.zoom(zoom_factor);
            sf::Vector2f after = window.mapPixelToCoords(mouse_px, grid_view);
            grid_view.move(before - after);
            
            continue;
        }
        
        // mouse move
        if (const auto* move = event->getIf<sf::Event::MouseMoved>()) {
            
            if (io.WantCaptureMouse) { continue; }

            sf::Vector2i mouse_px {move->position.x, move->position.y};
            sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);
            Point p = engine.snap_to_grid({mouse_world.x, mouse_world.y});

            if (placing_opening) {

                const Wall& w = engine.get_walls()[opening_wall_index];
                double t = project_t_onto_wall(p, w);
                preview_point = lerp_point(w.a, w.b, t);
                continue;

            }

            if (!drawing) { continue; }
            preview_point = p;
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {

            if (io.WantCaptureKeyboard) {
                if (
                    key->code != sf::Keyboard::Key::Left &&
                    key->code != sf::Keyboard::Key::Right &&
                    key->code != sf::Keyboard::Key::Up &&
                    key->code != sf::Keyboard::Key::Down &&
                    key->code != sf::Keyboard::Key::Escape &&
                    key->code != sf::Keyboard::Key::LShift
                ) {
                    continue;
                }
            }
            
            // escape key = cancel drawing wall
            if (key->code == sf::Keyboard::Key::Escape) {

                if (drawing) {
                    drawing = false; // cancel drawing
                    preview_point = start_point;
                }
            }
            
            // arrow keys to move around in select mode
            // when pressing left shift, move step size is bigger
            if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::Down) {
                // only allow shifting in select mode
                if (interaction_mode != InteractionMode::Select) {
                    continue;
                }

                double step = engine.get_cm_per_cell();
                if (key->shift) {
                    step *= SHIFT_MOVE_MULTIPLIER;
                }

                double dx = 0.0; 
                double dy = 0.0;

                switch (key->code) {
                    case sf::Keyboard::Key::Left: dx = -step; break;
                    case sf::Keyboard::Key::Right: dx = step; break;
                    case sf::Keyboard::Key::Up: dy = -step; break;
                    case sf::Keyboard::Key::Down: dy = step; break;
                    default: break;
                }

                // clear all selections and cancel interactions
                selection.clear();
                drawing = false;
                placing_opening = false;

                undo_stack.execute(std::make_unique<ShiftGeometryCommand>(engine, dx, dy));

                continue;
            }
        }

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (io.WantCaptureMouse) { continue; }

            if (app_state.mode == AppMode::NewProjectSetup) {
                if (mouse->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mouse_px{mouse->position.x, mouse->position.y};
                    sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);
                    Point p = engine.snap_to_grid({mouse_world.x, mouse_world.y});

                    if (!scale_has_p1) {
                        scale_p1 = p;
                        scale_has_p1 = true;
                    } else if (!scale_has_p2) {
                        scale_p2 = p;
                        scale_has_p2 = true;
                    }
                }
                return;
            }

            if (mouse->button == sf::Mouse::Button::Left) {

                // stop editing once finalized                
                if (blueprint_finalized) { return; }

                sf::Vector2i mouse_px{mouse->position.x, mouse->position.y};
                sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);
                Point p = engine.snap_to_grid({mouse_world.x, mouse_world.y});

                if (interaction_mode == InteractionMode::Draw && (current_tool == Tool::PlaceDoor || current_tool == Tool::PlaceWindow || current_tool == Tool::PlaceOpen)) {

                    if (placing_opening) { 
                        double len = distance_cm(start_point, preview_point);
                        if (len > 1.0) {
                            const Wall& w = engine.get_walls()[opening_wall_index];
                            Point mid{(start_point.x_cm + preview_point.x_cm) * 0.5, (start_point.y_cm + preview_point.y_cm) * 0.5};
                            WallOpening o;
                            o.center_t = project_t_onto_wall(mid, w);
                            o.length_cm = len;
                            o.type = (current_tool == Tool::PlaceDoor) ? ::OpeningType::Door : (current_tool == Tool::PlaceWindow) ? ::OpeningType::Window : ::OpeningType::Open;
                            auto& openings = engine.get_walls_mutable()[opening_wall_index].openings;
                            std::size_t opening_index = openings.size();
                            undo_stack.execute(std::make_unique<AddOpeningCommand>(engine, opening_wall_index, opening_index, o));
                        }
                        placing_opening = false;
                        return;
                    } 

                    double best_dist = pixel_radius_to_world_cm(PICK_RADIUS_PX);
                    std::optional<size_t> hit_wall;

                    for (size_t i = 0; i < engine.get_walls().size(); ++i) {
                        const auto& w = engine.get_walls()[i];
                        double d = distance_point_to_segment(p, w.a, w.b);
                        if (d < best_dist) {
                            best_dist = d;
                            hit_wall = i;
                        }
                    }

                    if (!hit_wall.has_value()) { return; } // didn't click on wall

                    opening_wall_index = *hit_wall;
                    opening_type = (current_tool == Tool::PlaceDoor) ? ::OpeningType::Door : (current_tool == Tool::PlaceWindow) ? ::OpeningType::Window : ::OpeningType::Open;
                    const Wall& w = engine.get_walls()[opening_wall_index];
                    double t = project_t_onto_wall(p, w);
                    start_point = lerp_point(w.a, w.b, t);
                    preview_point = start_point;
                    placing_opening = true;
                    drawing = false;
                    return;
                }

                if (interaction_mode == InteractionMode::Select) {
                    handle_select_click(p);
                    return;
                }

                if (current_tool == Tool::PlaceSource) {
                    PointEntity e;
                    e.position = p;
                    e.type = PointType::Source;
                    e.label = "";
                    e.source = SourceData{
                        .num_patients = 1.0f,                   // number of patients per week
                        .activity_per_patient_MBq = 0.0f,       // activity per patient
                        .uptake_time_hours = 0.0f,              // uptake time (hours)
                        .apply_patient_attenuation = true,      // (bool) apply radiation decay 
                        .apply_radioactive_decay = true         // (bool) apply radioactive decay
                    };
                    undo_stack.execute(std::make_unique<AddEntityCommand>(engine, e));
                    return;
                }

                if (current_tool == Tool::PlaceDose) {
                    PointEntity e;
                    e.position = p;
                    e.type = PointType::Dose;
                    e.label = "";
                    e.dose = DoseData{
                        .occupancy = 0.0f,                      // occupancy [0, 1] at dose point
                        .occupancy_type = "",                   // label (office, waiting room, staircase)
                        .dose_limit_uSv = 0.0f                  // dose limit at dose point
                    };
                    undo_stack.execute(std::make_unique<AddEntityCommand>(engine, e));
                    return;
                }

                if (placing_opening) {
                    return;
                }

                if (!drawing) {
                    start_point = p;
                    preview_point = p;
                    drawing = true;
                } else {
                    Wall wall;
                    wall.a = start_point;
                    wall.b = p;
                    wall.layers.push_back({1, 10.0});
                    wall.length_cm = std::hypot(wall.a.x_cm - wall.b.x_cm, wall.a.y_cm - wall.b.y_cm);
                    undo_stack.execute(std::make_unique<AddWallCommand>(engine, wall));
                    drawing = false;
                }
            }
        }
    }
}


void GridRenderer::render_grid_only() {
    window.setView(grid_view);

    float pixels_per_cm = static_cast<float>(window.getSize().x) / grid_view.getSize().x;
    length_text.setCharacterSize(static_cast<unsigned>(18.f / pixels_per_cm));

    if (background_sprite) {
        window.draw(*background_sprite);
    }

    const auto& bounds = engine.get_world_bounds();
    const double min_x = 0.0;
    const double max_x = bounds.width_cm;
    const double min_y = 0.0;
    const double max_y = bounds.height_cm;
    const double grid_spacing_cm = engine.get_cm_per_cell();

    for (double x = min_x; x <= max_x; x += grid_spacing_cm) {
        sf::Vertex line[2] = {
            {{(float)x, (float)min_y}, Cosmetics::GRID_COLOR},
            {{(float)x, (float)max_y}, Cosmetics::GRID_COLOR}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    for (double y = min_y; y <= max_y; y += grid_spacing_cm) {
        sf::Vertex line[2] = {
            {{(float)min_x, (float)y}, Cosmetics::GRID_COLOR},
            {{(float)max_x, (float)y}, Cosmetics::GRID_COLOR}
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    if (app_state.mode == AppMode::NewProjectSetup) {
        
        float r = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::POINT_RADIUS_PX));
        if (scale_has_p1) {
            sf::CircleShape p1;
            p1.setRadius(r);
            p1.setOrigin(sf::Vector2f{r, r});
            p1.setPosition(sf::Vector2f{static_cast<float>(scale_p1.x_cm), static_cast<float>(scale_p1.y_cm)});
            p1.setFillColor(sf::Color::Red);
            window.draw(p1);
        }
        if (scale_has_p2) {
            sf::CircleShape p2;
            p2.setRadius(r);
            p2.setOrigin(sf::Vector2f{r, r});
            p2.setPosition(sf::Vector2f{static_cast<float>(scale_p2.x_cm), static_cast<float>(scale_p2.y_cm)});
            p2.setFillColor(sf::Color::Red);
            window.draw(p2);

            sf::Vertex line[2];
            line[0].position = sf::Vector2f{static_cast<float>(scale_p1.x_cm), static_cast<float>(scale_p1.y_cm)};
            line[1].position = sf::Vector2f{static_cast<float>(scale_p2.x_cm), static_cast<float>(scale_p2.y_cm)};
            line[0].color = line[1].color = sf::Color::Red;
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }
}


void GridRenderer::render() {    
    // 1. project picker mode
    if (app_state.mode == AppMode::ProjectPicker) {
        window.setView(window.getDefaultView());
        window.clear(sf::Color::White);
        draw_project_picker();
        ImGui::SFML::Render(window);
        window.display();
        return;
    }
    
    // 2. new project setup
    if (app_state.mode == AppMode::NewProjectSetup) {
        render_grid_only();    
        draw_new_project_setup();
        ImGui::SFML::Render(window);
        window.display();
        return;
    }

    // 3. Editor

    window.setView(grid_view);
    float pixels_per_cm = static_cast<float>(window.getSize().x) / grid_view.getSize().x;
    length_text.setCharacterSize(static_cast<unsigned>(18.f / pixels_per_cm));

    if (background_sprite) {
        window.draw(*background_sprite);
    }

    // draw grid
    const auto& bounds = engine.get_world_bounds();
    const double min_x = 0.0;
    const double max_x = bounds.width_cm;
    const double min_y = 0.0;
    const double max_y = bounds.height_cm;
    const double grid_spacing_cm = engine.get_cm_per_cell();
    
    for (double x = min_x; x <= max_x; x += grid_spacing_cm) {

        sf::Vertex line[2];
        line[0].position = sf::Vector2f{static_cast<float>(x), static_cast<float>(min_y)};
        line[1].position = sf::Vector2f{static_cast<float>(x), static_cast<float>(max_y)};
        line[0].color = Cosmetics::GRID_COLOR;
        line[1].color = Cosmetics::GRID_COLOR;
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    for (double y = min_y; y <= max_y; y += grid_spacing_cm) {

        sf::Vertex line[2];
        line[0].position = sf::Vector2f{static_cast<float>(min_x), static_cast<float>(y)};
        line[1].position = sf::Vector2f{static_cast<float>(max_x), static_cast<float>(y)};
        line[0].color = Cosmetics::GRID_COLOR;
        line[1].color = Cosmetics::GRID_COLOR;
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    if (drawing) {
        float wall_thickness_cm = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::WALL_THICKNESS_PX));
        auto preview_rect = make_thick_segment(
                {static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)},
                {static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)},
                wall_thickness_cm,
                Cosmetics::WALL_NORMAL
        );
        window.draw(preview_rect);
        double len_cm = distance_cm(start_point, preview_point) * engine.get_distance_scale();
        sf::Vector2f mid{static_cast<float>((start_point.x_cm + preview_point.x_cm) * 0.5), static_cast<float>((start_point.y_cm + preview_point.y_cm) * 0.5)};
        
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << len_cm << " cm";
        length_text.setString(ss.str());
        length_text.setPosition(mid);
        window.draw(length_text);
    }

    // draw walls
    const auto& walls = engine.get_walls();
    for (std::size_t i = 0; i < walls.size(); ++i) {
        const auto& w = walls[i];
        sf::Color wall_color = (selection.type == Selection::Type::Wall && selection.wall_index == i) ? Cosmetics::WALL_SELECTED : Cosmetics::WALL_NORMAL;
        float wall_thickness_cm = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::WALL_THICKNESS_PX));
        auto wall_rect = make_thick_segment(
                {static_cast<float>(w.a.x_cm), static_cast<float>(w.a.y_cm)},
                {static_cast<float>(w.b.x_cm), static_cast<float>(w.b.y_cm)},
                wall_thickness_cm,
                wall_color
        );
        window.draw(wall_rect);
    
        if (placing_opening && i == opening_wall_index) {
            sf::Vertex preview[2];
            preview[0].position = sf::Vector2f{static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)};
            preview[1].position = sf::Vector2f{static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)};
            sf::Color c;
            switch (opening_type) {
                case ::OpeningType::Door: c = Cosmetics::DOOR_COLOR; break;
                case ::OpeningType::Window: c = Cosmetics::WINDOW_COLOR; break;
                case ::OpeningType::Open: c = Cosmetics::OPEN_COLOR; break;
            }
            float opening_thickness_cm = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::OPENING_THICKNESS_PX));
            auto rect = make_thick_segment(
                                        {static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)},
                                        {static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)},
                                        opening_thickness_cm,
                                        c
                                    );
            window.draw(rect);

            // draw text
            double len_cm = distance_cm(start_point, preview_point) * engine.get_distance_scale();
            sf::Vector2f mid{
                static_cast<float>((start_point.x_cm + preview_point.x_cm) * 0.5), 
                static_cast<float>((start_point.y_cm + preview_point.y_cm) * 0.5)
            };

            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << len_cm << " cm";
            length_text.setString(ss.str());

            // color matches opening type on top of wall
            switch (opening_type) {
                case ::OpeningType::Door: length_text.setFillColor(Cosmetics::DOOR_TEXT_COLOR); break;
                case ::OpeningType::Window: length_text.setFillColor(Cosmetics::WINDOW_TEXT_COLOR); break;
                case ::OpeningType::Open: length_text.setFillColor(Cosmetics::OPEN_TEXT_COLOR); break;
            }

            length_text.setPosition(mid);
            window.draw(length_text);
        }

        for (const auto& o : w.openings) {

            double half_t = (o.length_cm * 0.5) / w.length_cm;
            double t0 = std::clamp(o.center_t - half_t, 0.0, 1.0);
            double t1 = std::clamp(o.center_t + half_t, 0.0, 1.0);

            Point p0 = lerp_point(w.a, w.b, t0);
            Point p1 = lerp_point(w.a, w.b, t1);

            sf::Vertex opening[2];
            opening[0].position = {static_cast<float>(p0.x_cm), static_cast<float>(p0.y_cm)};
            opening[1].position = {static_cast<float>(p1.x_cm), static_cast<float>(p1.y_cm)};

            sf::Color c;

            if (selection.type == Selection::Type::Opening && selection.wall_index == i && selection.opening_index == (&o - &w.openings[0])) {
                c = Cosmetics::WALL_SELECTED;
            } else {
                switch (o.type) {
                    case ::OpeningType::Door: c = Cosmetics::DOOR_COLOR; break;
                    case ::OpeningType::Window: c = Cosmetics::WINDOW_COLOR; break;
                    case ::OpeningType::Open: c = Cosmetics::OPEN_COLOR; break;
                }
            }

            float opening_thickness_cm = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::OPENING_THICKNESS_PX));
            auto rect = make_thick_segment(
                                        {static_cast<float>(p0.x_cm), static_cast<float>(p0.y_cm)},
                                        {static_cast<float>(p1.x_cm), static_cast<float>(p1.y_cm)},
                                        opening_thickness_cm,
                                        c
                                    );
            window.draw(rect);

            // place text
            sf::Vector2f mid{static_cast<float>((p0.x_cm + p1.x_cm) * 0.5), static_cast<float>((p0.y_cm + p1.y_cm) * 0.5)};
            sf::Vector2f direction{static_cast<float>(p1.x_cm - p0.x_cm), static_cast<float>(p1.y_cm - p0.y_cm)};
            float mag = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (mag > 0.f) {
                direction /= mag;
            }

            // perpendicular offset so text sits beside opening once placed
            sf::Vector2f normal{-direction.y, direction.x};
            if (std::abs(direction.x) > std::abs(direction.y)) {
                normal = sf::Vector2f{direction.y, -direction.x};
            }
            float opening_offset_cm = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::OPENING_THICKNESS_PX));
            sf::Vector2f text_position = mid + normal * opening_offset_cm;

            // actual text
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << (o.length_cm * engine.get_distance_scale()) << " cm";
            length_text.setString(ss.str());

            switch (o.type) {
                case ::OpeningType::Door: length_text.setFillColor(Cosmetics::DOOR_TEXT_COLOR); break;
                case ::OpeningType::Window: length_text.setFillColor(Cosmetics::WINDOW_TEXT_COLOR); break;
                case ::OpeningType::Open: length_text.setFillColor(Cosmetics::OPEN_TEXT_COLOR); break;
            }

            if (selection.type == Selection::Type::Opening && selection.wall_index == i && selection.opening_index == (&o - &w.openings[0])) {
                length_text.setFillColor(Cosmetics::WALL_SELECTED);
            }

            length_text.setPosition(text_position);
            window.draw(length_text);
        }

        const Point mid{(w.a.x_cm + w.b.x_cm) * 0.5, (w.a.y_cm + w.b.y_cm) * 0.5};

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << (w.length_cm * engine.get_distance_scale()) << " cm";
        length_text.setFillColor(Cosmetics::LENGTH_TEXT_COLOR);
        length_text.setString(ss.str());
        length_text.setPosition(sf::Vector2f{static_cast<float>(mid.x_cm), static_cast<float>(mid.y_cm)});

        window.draw(length_text);
    }

    // draw source + dose points
    const auto& entities = engine.get_entities();
    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto& e = entities[i];
        sf::CircleShape marker;
        float point_radius_cm = static_cast<float>(pixel_radius_to_world_cm(Cosmetics::POINT_RADIUS_PX));
        marker.setRadius(point_radius_cm);
        marker.setOrigin(sf::Vector2f{point_radius_cm, point_radius_cm});
        marker.setPosition(sf::Vector2f{static_cast<float>(e.position.x_cm), static_cast<float>(e.position.y_cm)});
        const bool selected = selection.type == Selection::Type::Entity && selection.entity_index == i;

        if (e.type == PointType::Source) {
            marker.setFillColor(selected ? Cosmetics::SOURCE_SELECTED_COLOR : Cosmetics::SOURCE_COLOR);
        } else {
            marker.setFillColor(sf::Color::Transparent);
            marker.setOutlineThickness(static_cast<float>(pixel_radius_to_world_cm(Cosmetics::POINT_OUTLINE_THICKNESS_PX)));
            marker.setOutlineColor(selected ? Cosmetics::DOSE_SELECTED_COLOR : Cosmetics::DOSE_COLOR);
        }

        window.draw(marker);
    } 

    // temporary panel view
    window.setView(window.getDefaultView());
    draw_toolbar();
    draw_left_panel();

    ImGui::SetNextWindowPos(ImVec2(window.getSize().x - RIGHT_PANEL_WIDTH_PX, TOOLBAR_HEIGHT_PX));
    ImGui::SetNextWindowSize(ImVec2(RIGHT_PANEL_WIDTH_PX, window.getSize().y - TOOLBAR_HEIGHT_PX));
    ImGui::Begin("Terminal", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::BeginChild("terminal_scroll");
    for (const auto& line : ui_log.lines) {
        ImGui::TextUnformatted(line.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();

}