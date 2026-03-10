// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <imgui.h>
#include <imgui-SFML.h>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "ui/GridRenderer.hpp"
#include "materials/MaterialRegistry.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "ui/GridMath.hpp"
#include "ui/AddWallCommand.hpp"
#include "ui/DeleteWallCommand.hpp"
#include "ui/RemoveEntityCommand.hpp"
#include "ui/AddEntityCommand.hpp"
#include "ui/ShiftGeometryCommand.hpp"
#include "ui/AddWallLayerCommand.hpp"
#include "ui/RemoveWallLayerCommand.hpp"
#include "ui/EditWallLayerCommand.hpp"
#include "ui/UiLog.hpp"
#include "ui/Cosmetics.hpp"
#include "calc/SceneCompiler.hpp"
#include "calc/CompilerOutput.hpp"
#include "optimization/ShieldOptimizer.hpp"
#include "output/PrintCompilerOutput.hpp"
#include "output/PrintCompilerOutputUI.hpp"
#include "output/ExportCompilerOutputCSV.hpp"
#include "utils/AppPaths.hpp"
#include "utils/PdfToPng.hpp"
#include "utils/ProjectIO.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"


using namespace geom;
using namespace app;
using namespace utils;
using namespace output;
using namespace isotope;
using namespace material;

extern MaterialRegistry material_registry;
extern IsotopeRegistry isotope_registry;


namespace { // anonymous

    // interaction constants
    constexpr double SELECT_EPS_CM = 25.0;          // select wall/point by clicking within 25cm of it
    constexpr double SHIFT_MOVE_MULTIPLIER = 10.0;  // when shift is pressed, the drawing shifts SHIFT_MOVE_MULTIPLIER times the normal distance
    
    // ui layout constants
    constexpr float TOOLBAR_HEIGHT_PX = 40.f;
    constexpr float TOOLBAR_GROUP_GAP_PX = 6.f;
    constexpr float TOOLBAR_CLUSTER_PAD_X = 3.f;
    constexpr float TOOLBAR_CLUSTER_PAD_Y = 2.f;
    constexpr float TOOLBAR_CLUSTER_ROUNDING = 3.f;
    constexpr float SIDE_PANEL_WIDTH_RATIO = 0.22f;
    constexpr float SIDE_PANEL_MIN_WIDTH_PX = 220.f;
    constexpr float SIDE_PANEL_MAX_WIDTH_PX = 320.f;

    float side_panel_width_px(const sf::RenderWindow& window) {
        const float width = static_cast<float>(window.getSize().x);
        return std::clamp(width * SIDE_PANEL_WIDTH_RATIO, SIDE_PANEL_MIN_WIDTH_PX, SIDE_PANEL_MAX_WIDTH_PX);
    }

    // used to be static method
    bool ToolbarButton(const char* label, bool active) {
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        bool clicked = ImGui::Button(label);

        if (active) {
            ImGui::PopStyleColor(3);
        }

        return clicked;
    }

    void ToolbarTooltip(const char* text) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", text);
        }
    }

    void DrawHelpButtonPreview(const char* label) {
        ImGui::BeginDisabled();
        ImGui::Button(label);
        ImGui::EndDisabled();
    }

    void ToolbarGap() {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(TOOLBAR_GROUP_GAP_PX, 0.f));
        ImGui::SameLine();
    }

    std::string fmt_compact(double value, int precision = 2) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return ss.str();
    }

    std::string build_wall_layers_label(const Wall& wall) {
        std::ostringstream ss;
        bool first = true;

        for (const auto& layer : wall.layers) {
            if (layer.thickness_cm <= 1e-6) {
                continue;
            }

            const MaterialDef* mat = material_registry.get(layer.material_id);
            const std::string material_name = mat ? mat->name : ("Material " + std::to_string(layer.material_id));

            if (!first) {
                ss << " | ";
            }

            ss << material_name << " " << fmt_compact(layer.thickness_cm) << " cm";
            first = false;
        }

        return ss.str();
    }

} // end of anonymous namespace


namespace ui {

    GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e, app::AppState& state) : window(w), engine(e), app_state(state), length_text(font) {
        
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
        if (!font.openFromFile(asset_path("fonts/Inter-Regular.ttf").string())) {
            std::cerr << "Failed to load font\n";
        }
        if (!shieldlabs_logo.loadFromFile(asset_path("logos/ShieldLabsTitleLogoTransparent.png").string())) {
            std::cerr << "Failed to load logo\n"; 
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
        const float side_panel_px = side_panel_width_px(window);
        const float side_norm = side_panel_px / win_w;
        const float avail_x = side_norm;
        const float avail_y = top_norm;
        const float avail_w = 1.f - 2.f * side_norm;
        const float avail_h = 1.f - top_norm;

        if (avail_w <= 0.f || avail_h <= 0.f) { return; }

        const float avail_px_w = avail_w * win_w;
        const float avail_px_h = avail_h * win_h;

        sf::FloatRect viewport;
        viewport.position.x = avail_x;
        viewport.position.y = avail_y;
        viewport.size.x = avail_w;
        viewport.size.y = avail_h;

        grid_view.setViewport(viewport);
        // Keep square grid cells and fit the full world bounds into the available area.
        const auto& bounds = engine.get_world_bounds();
        const float world_w = static_cast<float>(bounds.width_cm);
        const float world_h = static_cast<float>(bounds.height_cm);
        if (world_w <= 0.f || world_h <= 0.f) { return; }

        const float units_per_px = std::max(world_w / avail_px_w, world_h / avail_px_h);
        sf::Vector2f view_size{units_per_px * avail_px_w, units_per_px * avail_px_h};
        grid_view.setSize(view_size);
        grid_view.setCenter(sf::Vector2f{world_w * 0.5f, world_h * 0.5f});
    }


    bool GridRenderer::load_background_image(const std::string& path)
    {
        background_sprite.reset();
        background_texture = sf::Texture{};

        if (!background_texture.loadFromFile(path)) {
            std::cerr << "Failed to load background image: " << path << "\n";
            return false;
        }

        const auto size_px = background_texture.getSize();

        engine.set_world_bounds_from_image(size_px.x, size_px.y);
        const auto& bounds = engine.get_world_bounds();

        grid_view.setSize({
            static_cast<float>(bounds.width_cm),
            static_cast<float>(bounds.height_cm)
        });

        grid_view.setCenter(sf::Vector2f{static_cast<float>(bounds.width_cm * 0.5), static_cast<float>(bounds.height_cm * 0.5)});

        update_viewport();

        background_sprite = std::make_unique<sf::Sprite>(background_texture);
        background_sprite->setPosition({0.f, 0.f});

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
            ui_log.push("[VALIDATION PASSED]");
        }

        blueprint_finalized = !blueprint_finalized;
        drawing = false;
        selection.clear();

        if (blueprint_finalized) {
            ui_log.clear();
            ui_log.push("[Grid Editing Locked]");
            ui_log.push("Dose Calculation Results:");
            // canonical JSON
            nlohmann::json j = engine.to_json();
            // uncomment to dump json to application terminal for debugging
            // ui_log.push(j.dump(2));
            // compile
            calc::CalcScene scene = calc::SceneCompiler::compile(j);
            ui_log.separator();
            ui_log.push("[Captured Geometric Entities]");
            ui_log.push("- Sources: " + std::to_string(scene.sources.size()));
            ui_log.push("- Dose Points: " + std::to_string(scene.dose_points.size()));
            ui_log.push("- Walls: " + std::to_string(scene.walls.size()));
            calc::CompilerOutput compiler_output = calc::build_compiler_output(scene);
            ui_log.separator();
            print_to_ui(compiler_output, ui_log);
            // uncomment for terminal debugging
            // output::print(compiler_output);
            last_compiler_output = compiler_output;
        } else {
            ui_log.clear();
            ui_log.push("[Grid Editing Unlocked]");
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

    void GridRenderer::draw_project_picker()
    {
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)), ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(11.f / 255.f, 11.f / 255.f, 20.f / 255.f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("ShieldLabs", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 bg_min = ImGui::GetWindowPos();
        const ImVec2 bg_max(bg_min.x + ImGui::GetWindowWidth(), bg_min.y + ImGui::GetWindowHeight());
        draw_list->AddRectFilledMultiColor(
            bg_min,
            bg_max,
            IM_COL32(5, 8, 18, 255),
            IM_COL32(8, 12, 24, 255),
            IM_COL32(3, 5, 14, 255),
            IM_COL32(5, 8, 18, 255)
        );

        const float panel_width = std::min(980.f, ImGui::GetWindowWidth() - 120.f);
        const float panel_height = 340.f;
        const ImVec2 panel_pos(
            (ImGui::GetWindowWidth() - panel_width) * 0.5f,
            (ImGui::GetWindowHeight() - panel_height) * 0.5f
        );
        const ImVec2 panel_screen_min(bg_min.x + panel_pos.x, bg_min.y + panel_pos.y);
        const ImVec2 panel_screen_max(panel_screen_min.x + panel_width, panel_screen_min.y + panel_height);
        const ImU32 panel_ghost = IM_COL32(110, 160, 220, 28);
        const ImU32 panel_ghost_strong = IM_COL32(135, 190, 245, 38);
        const ImVec2 panel_offset(24.f, -16.f);
        const ImVec2 panel_back_min(panel_screen_min.x + panel_offset.x, panel_screen_min.y + panel_offset.y);
        const ImVec2 panel_back_max(panel_screen_max.x + panel_offset.x, panel_screen_max.y + panel_offset.y);
        draw_list->AddRect(panel_back_min, panel_back_max, panel_ghost, 0.f, 0, 1.0f);
        draw_list->AddRect(panel_screen_min, panel_screen_max, panel_ghost, 0.f, 0, 1.0f);
        draw_list->AddLine(panel_screen_min, panel_back_min, panel_ghost_strong, 1.0f);
        draw_list->AddLine(ImVec2(panel_screen_max.x, panel_screen_min.y), ImVec2(panel_back_max.x, panel_back_min.y), panel_ghost_strong, 1.0f);
        draw_list->AddLine(ImVec2(panel_screen_min.x, panel_screen_max.y), ImVec2(panel_back_min.x, panel_back_max.y), panel_ghost_strong, 1.0f);
        draw_list->AddLine(panel_screen_max, panel_back_max, panel_ghost_strong, 1.0f);

        ImGui::SetCursorPos(panel_pos);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.12f, 0.17f, 0.96f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.28f, 0.33f, 0.42f, 0.80f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.f, 24.f));
        ImGui::BeginChild("LaunchPanel", ImVec2(panel_width, panel_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.15f, 0.21f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.31f, 0.40f, 0.90f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.f, 12.f));
        ImGui::BeginChild("LaunchHeader", ImVec2(-1.f, 104.f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        static constexpr const char* launch_banner[] = {
            "     _____ __  ________________   ____  __    ___    ____  _____",
            "    / ___// / / /  _/ ____/ / /  / __ \\/ /   /   |  / __ // ___/",
            "    \\__ \\/ /_/ // // __/ / /    / / / / /   / /| | / __  |\\__ \\",
            "   ___/ / __  // // /___/ /___ / /_/ / /___/ ___ |/ /_/ /___/ /",
            "  /____/_/ /_/___/_____/_____//_____/_____/_/  |_/____//____/"
        };
        const float header_content_width = ImGui::GetContentRegionAvail().x;
        const float line_height = ImGui::GetTextLineHeightWithSpacing();
        const float banner_height = line_height * 5.0f;
        const float banner_start_y = std::max(0.f, (104.f - banner_height) * 0.5f - 2.f);
        const float time = static_cast<float>(ImGui::GetTime());
        const float pulse = 0.88f + 0.12f * std::sin(time * 1.8f);
        const float mix = 0.5f + 0.5f * std::sin(time * 1.1f);
        const ImVec4 cool_a(0.72f, 0.80f, 0.96f, 1.0f);
        const ImVec4 cool_b(0.58f, 0.90f, 0.92f, 1.0f);
        const ImVec4 banner_color(
            (cool_a.x + (cool_b.x - cool_a.x) * mix) * pulse,
            (cool_a.y + (cool_b.y - cool_a.y) * mix) * pulse,
            (cool_a.z + (cool_b.z - cool_a.z) * mix) * pulse,
            1.0f
        );

        for (int i = 0; i < 5; ++i) {
            const float line_width = ImGui::CalcTextSize(launch_banner[i]).x;
            const float line_x = std::max(0.f, (header_content_width - line_width) * 0.5f);
            ImGui::SetCursorPos(ImVec2(line_x, banner_start_y + i * line_height));
            ImGui::TextColored(banner_color, "%s", launch_banner[i]);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        ImGui::Spacing();

        const float content_width = ImGui::GetContentRegionAvail().x;
        const float gap = 18.f;
        const float card_width = (content_width - gap) * 0.5f;
        const float card_height = 180.f;

        const auto draw_action_card = [&](const char* id, const char* title, const char* description, const char* button_label) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.16f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.35f, 0.45f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.f, 16.f));
            ImGui::BeginChild(id, ImVec2(card_width, card_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::Text("%s", title);
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextColored(ImVec4(0.82f, 0.85f, 0.90f, 1.0f), "%s", description);
            ImGui::PopTextWrapPos();
            ImGui::SetCursorPosY(card_height - 62.f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27f, 0.31f, 0.38f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.33f, 0.38f, 0.46f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.26f, 0.33f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.f, 12.f));
            bool clicked = ImGui::Button(button_label, ImVec2(-1.f, 36.f));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            return clicked;
        };

        if (draw_action_card(
                "NewProjectCard",
                "New Project",
                "Set up a new shielding project from a floorplan PDF and calibrate the plan before editing geometry.",
                "Create New Project")) {
            app_state.mode = AppMode::NewProjectSetup;
            scale_has_p1 = false;
            scale_has_p2 = false;
            update_viewport();
        }

        ImGui::SameLine(0.f, gap);

        if (draw_action_card(
                "OpenProjectCard",
                "Open Project",
                "Load an existing ShieldLabs project to continue geometry edits, calculations, and optimization work.",
                "Open Existing Project")) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog(
                "OpenProject",
                "Open Project",
                ".json",
                config
            );

            app_state.mode = AppMode::OpeningProject;
        }

        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }


    void GridRenderer::draw_new_project_setup() {
        const float panel_width = side_panel_width_px(window);
        ImGui::SetNextWindowPos(ImVec2(10, TOOLBAR_HEIGHT_PX + 10));
        ImGui::SetNextWindowSize(ImVec2(panel_width - 20.f, window.getSize().y - TOOLBAR_HEIGHT_PX - 20.f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.13f, 0.14f, 0.17f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.28f, 0.33f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.31f, 0.35f, 0.41f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.21f, 0.24f, 0.29f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.30f, 0.37f, 0.72f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 14.f));
        ImGui::Begin("##ProjectSetup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::Text("Project Setup");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
        ImGui::TextDisabled("Load the floorplan, then calibrate the scale.");
        ImGui::PopTextWrapPos();
        ImGui::Spacing();

        ImGui::Text("Floorplan");
        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("Upload PDF Floorplan", ImVec2(-1.f, 0.f))) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog(
                "ChoosePDF",
                "Select PDF",
                ".pdf",
                config
            );
        }

        if (!pdf_error_message.empty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.20f, 0.15f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.52f, 0.27f, 0.27f, 0.70f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 10.f));
            ImGui::BeginChild("FloorplanError", ImVec2(0.f, 96.f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::TextColored(ImVec4(0.85f, 0.30f, 0.30f, 1.0f), "Floorplan Error");
            ImGui::TextWrapped("%s", pdf_error_message.c_str());
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        ImGui::Spacing();
        ImGui::Text("Scale Calibration");
        ImGui::Separator();
        ImGui::TextWrapped("Click two points on the grid that represent a known real-world distance.");
        ImGui::Spacing();
        ImGui::Text("Real-world distance (cm)");
        ImGui::SetNextItemWidth(-1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.22f, 0.20f, 0.22f, 1.0f));
        ImGui::InputDouble("##scale_cm", &scale_real_distance_cm, 10.0, 100.0);
        ImGui::PopStyleColor();

        if (scale_has_p1 && scale_has_p2) {
            double pixel_dist = distance_cm(scale_p1, scale_p2);
            ImGui::Spacing();
            ImGui::Text("Measured distance: %.2f grid units", pixel_dist);
            ImGui::TextWrapped(
                "Grid units are arbitrary until calibrated. "
                "Press 'Apply Scale' to calibrate and proceed to editing. "
                "If you would like to re-calibrate the scale once entering Editing, press the 'Edit Scale' button to return to this page. "
            );
            ImGui::Spacing();
            if (ImGui::Button("Apply Scale", ImVec2(-1.f, 0.f))) {
                double measured_draw_distance = distance_cm(scale_p1, scale_p2);
                if (measured_draw_distance > 0.0) {
                    engine.set_distance_scale(scale_real_distance_cm / measured_draw_distance);
                }

                update_viewport();
                app_state.mode = AppMode::Editing;
            }
        } else {
            ImGui::Spacing();
            ImGui::TextDisabled("Select two points on the grid.");
        }

        const float reset_button_height = ImGui::GetFrameHeight();
        const float bottom_gap = 14.f;
        const float reset_y = ImGui::GetWindowHeight() - reset_button_height - bottom_gap - ImGui::GetStyle().WindowPadding.y;
        if (ImGui::GetCursorPosY() < reset_y) {
            ImGui::SetCursorPosY(reset_y);
        } else {
            ImGui::Spacing();
        }

        if (ImGui::Button("Reset Points", ImVec2(-1.f, 0.f))) {
            scale_has_p1 = false;
            scale_has_p2 = false;
        }

        if (ImGuiFileDialog::Instance()->Display("ChoosePDF")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string pdf_path = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string png_path = "cache/blueprint.png";
                std::string pdf_load_error;

                if (pdf_to_png(pdf_path, png_path, 150, &pdf_load_error)) {
                    pdf_error_message.clear();
                    current_floorplan_png_path = png_path;
                    load_background_image(png_path);
                    scale_has_p1 = false;
                    scale_has_p2 = false;
                    update_viewport();
                } else {
                    pdf_error_message = 
                    "PDF upload failed.\n"
                    + pdf_load_error
                    + "\n\nRequirements:\n"
                    "- Must be exactly 1 page\n"
                    "- Must not be encrypted\n"
                    "- Must contain visible content";
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(5);
    }


    void GridRenderer::draw_toolbar() {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, TOOLBAR_HEIGHT_PX));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.13f, 0.14f, 0.17f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, 4.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.f, 6.f));
        ImGui::Begin("TopToolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->ChannelsSplit(2);
        draw_list->ChannelsSetCurrent(1);

        const auto push_button_group = [](const ImVec4& button, const ImVec4& hovered, const ImVec4& active) {
            ImGui::PushStyleColor(ImGuiCol_Button, button);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
        };

        const auto pop_button_group = []() { ImGui::PopStyleColor(3); };

        const auto push_menu_group = [&](const ImVec4& button, const ImVec4& hovered, const ImVec4& active) {
            push_button_group(button, hovered, active);
            ImGui::PushStyleColor(ImGuiCol_Header, button);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hovered);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, active);
        };

        const auto pop_menu_group = []() { ImGui::PopStyleColor(6); };

        const ImVec4 grey_button{0.38f, 0.40f, 0.44f, 1.0f};
        const ImVec4 grey_hover{0.45f, 0.47f, 0.51f, 1.0f};
        const ImVec4 grey_active{0.30f, 0.32f, 0.36f, 1.0f};
        const ImU32 cluster_fill = IM_COL32(28, 32, 40, 235);
        const ImU32 cluster_border = IM_COL32(64, 71, 83, 255);

        const auto draw_cluster_plate = [&](const ImVec2& min, const ImVec2& max) {
            draw_list->ChannelsSetCurrent(0);
            draw_list->AddRectFilled(
                ImVec2(min.x - TOOLBAR_CLUSTER_PAD_X, min.y - TOOLBAR_CLUSTER_PAD_Y),
                ImVec2(max.x + TOOLBAR_CLUSTER_PAD_X, max.y + TOOLBAR_CLUSTER_PAD_Y),
                cluster_fill,
                TOOLBAR_CLUSTER_ROUNDING
            );
            draw_list->AddRect(
                ImVec2(min.x - TOOLBAR_CLUSTER_PAD_X, min.y - TOOLBAR_CLUSTER_PAD_Y),
                ImVec2(max.x + TOOLBAR_CLUSTER_PAD_X, max.y + TOOLBAR_CLUSTER_PAD_Y),
                cluster_border,
                TOOLBAR_CLUSTER_ROUNDING
            );
            draw_list->ChannelsSetCurrent(1);
        };

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        if (ToolbarButton("Selection", interaction_mode == InteractionMode::Select)) {
            interaction_mode = InteractionMode::Select;
            current_tool = Tool::None;
            selection.clear();
            drawing = false;
        }
        ToolbarTooltip("Select and inspect geometry.");
        ImGui::SameLine();

        if (ToolbarButton("Editing", interaction_mode == InteractionMode::Draw)) {
            interaction_mode = InteractionMode::Draw;
            selection.clear();
            drawing = false;
        }
        ToolbarTooltip("Switch to placement and editing mode.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        if (ToolbarButton("Add Wall", current_tool == Tool::DrawWall)) {
            interaction_mode = InteractionMode::Draw;
            current_tool = Tool::DrawWall;
            drawing = false;
        }
        ToolbarTooltip("Place a wall segment.");
        ImGui::SameLine();

        if (ToolbarButton("Add Source", current_tool == Tool::PlaceSource)) {
            interaction_mode = InteractionMode::Draw;
            current_tool = Tool::PlaceSource;
        }
        ToolbarTooltip("Place a radiation source.");
        ImGui::SameLine();

        if (ToolbarButton("Add Dose", current_tool == Tool::PlaceDose)) {
            interaction_mode = InteractionMode::Draw;
            current_tool = Tool::PlaceDose;
        }
        ToolbarTooltip("Place a dose point.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        if (ToolbarButton("Undo", false)) {
            undo_stack.undo();
        }
        ToolbarTooltip("Reverse the last action.");
        ImGui::SameLine();

        if (ToolbarButton("Redo", false)) {
            undo_stack.redo();
        }
        ToolbarTooltip("Reapply the last undone action.");
        ImGui::SameLine();

        if (ToolbarButton("Delete", false)) {
            switch (selection.type) {

                case Selection::Type::Wall:
                    undo_stack.execute(std::make_unique<DeleteWallCommand>(engine, selection.wall_index));
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
        ToolbarTooltip("Remove the selected item.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        if (ToolbarButton("Isotopes", false)) {
            show_isotope_popup = true;
        }
        ToolbarTooltip("Show the list of supported isotopes.");
        ImGui::SameLine();

        if (ToolbarButton("Materials", false)) {
            show_material_popup = true;
        }
        ToolbarTooltip("Show the list of supported materials.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float toolbar_gap_width = TOOLBAR_GROUP_GAP_PX;
        float cluster_border_allowance = TOOLBAR_CLUSTER_PAD_X * 2.0f;
        float lock_w = ImGui::CalcTextSize("Run Calc").x + ImGui::GetStyle().FramePadding.x * 2;
        float unlock_w = ImGui::CalcTextSize("Unlock Geometry").x + ImGui::GetStyle().FramePadding.x * 2;
        float optimize_w = ImGui::CalcTextSize("Optimize").x + ImGui::GetStyle().FramePadding.x * 2;
        float show_w = ImGui::CalcTextSize("Results").x + ImGui::GetStyle().FramePadding.x * 2;
        float info_w = ImGui::CalcTextSize("Info").x + ImGui::GetStyle().FramePadding.x * 2;
        float edit_w = ImGui::CalcTextSize("Edit Scale").x + ImGui::GetStyle().FramePadding.x * 2;
        float save_w = ImGui::CalcTextSize("Save").x
            + ImGui::GetStyle().FramePadding.x * 2
            + ImGui::GetStyle().ItemInnerSpacing.x
            + ImGui::GetFontSize()
            + 10.0f;
        float help_w = ImGui::CalcTextSize("Help").x + ImGui::GetStyle().FramePadding.x * 2;

        float lock_group_w = lock_w + spacing + unlock_w + cluster_border_allowance;
        float optimize_group_w = optimize_w + spacing + show_w + spacing + info_w + cluster_border_allowance;
        float edit_group_w = edit_w + cluster_border_allowance;
        float save_group_w = save_w + cluster_border_allowance;
        float help_group_w = help_w + cluster_border_allowance;
        float total_w = lock_group_w
            + toolbar_gap_width
            + optimize_group_w
            + toolbar_gap_width
            + edit_group_w
            + toolbar_gap_width
            + help_group_w
            + toolbar_gap_width
            + save_group_w;
        float right_margin = 18.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - total_w - right_margin);

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        ImGui::BeginDisabled(blueprint_finalized);
        if (ToolbarButton("Run Calc", blueprint_finalized)) {
            finalize_blueprint(); // locks geometry
        }
        ToolbarTooltip("Calculate dose and lock geometry.");
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::BeginDisabled(!blueprint_finalized);
        if (ToolbarButton("Unlock Geometry", false)) {
            finalize_blueprint(); // unlocks geometry
        }
        ToolbarTooltip("Unlock geometry so you can edit again.");
        ImGui::EndDisabled();
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();
        
        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        ImGui::BeginDisabled(!blueprint_finalized || !last_compiler_output.has_value());
        if (ToolbarButton("Optimize", false)) {
            
            nlohmann::json j = engine.to_json();
            calc::CalcScene scene = calc::SceneCompiler::compile(j);
            
            auto before = calc::build_compiler_output(scene);
            
            optimization::ShieldOptimizer optimizer(scene);
            calc::CalcScene optimized_scene = optimizer.optimize();
            
            auto after = calc::build_compiler_output(optimized_scene);

            // cache results for visualization
            optimized_scene_cache = optimized_scene;
            optimized_output_cache = after;

            ui_log.push("");
            ui_log.push("[Shield Optimization]");
            bool any_violation = false;
            size_t n = std::min(before.dose_totals.size(), after.dose_totals.size());
            ui_log.push("[Dose Summary]");

            for (size_t i = 0; i < n; ++i) {
                const std::string& dose_name = after.dose_totals[i].dose_name.empty()
                    ? before.dose_totals[i].dose_name
                    : after.dose_totals[i].dose_name;
                double b = before.dose_totals[i].annual_dose_uSv;
                double a = after.dose_totals[i].annual_dose_uSv;
                double limit = after.dose_totals[i].dose_limit_uSv;
                bool violation = (a - limit > 1e-2);
                if (violation) {
                    any_violation = true;
                }
                 
                std::ostringstream ss;
                ss << "- "
                << (dose_name.empty() ? ("Dose Point " + std::to_string(i + 1)) : dose_name)
                << ": " << std::fixed << std::setprecision(2)
                << b << " -> " << a
                << " uSv/y (limit " << limit
                << ")";

                ui_log.push(ss.str());
            }

            ui_log.push("[Wall Updates]");
            bool any_wall_change = false;

            for (size_t i = 0; i < scene.walls.size(); ++i) {
                double old_lead = 0.0;
                double new_lead = 0.0;

                for (const auto& layer : scene.walls[i].layers) {
                    if (const auto* m = material_registry.get(layer.material_id)) {
                        if (m->key == "lead") old_lead = layer.thickness_cm;
                    }
                }

                for (const auto& layer : optimized_scene.walls[i].layers) {
                    if (const auto* m = material_registry.get(layer.material_id)) {
                        if (m->key == "lead") new_lead = layer.thickness_cm;
                    }
                }

                if (std::abs(new_lead - old_lead) > 1e-6) {
                    any_wall_change = true;

                    std::ostringstream ss;
                    ss << "- Wall " << (i + 1)
                    << ": lead "
                    << std::fixed << std::setprecision(2)
                    << old_lead << " -> " << new_lead << " cm";

                    ui_log.push(ss.str());
                }
            }

            if (!any_wall_change) {
                ui_log.push("No wall updates.");
            }

            ui_log.push(any_violation ? "[Dose Limits Not Met]" : "[All Dose Limits Met]");
            ui_log.push("Open [Results] for details");
            optimization_ran = true;
        }
        ToolbarTooltip("Optimize shielding.");
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::BeginDisabled(!optimized_scene_cache.has_value());
        if (ToolbarButton("Results", show_optimization_overlay)) {
            show_optimization_overlay = !show_optimization_overlay;
        }
        ToolbarTooltip("Show optimization results.");
        ImGui::EndDisabled();
        ImGui::SameLine();

        if (ToolbarButton("Info", show_display_info)) {
            show_display_info = !show_display_info;
        }
        ToolbarTooltip("Toggle wall, source, and dose annotations.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        if (ToolbarButton("Edit Scale", false)) {
            app_state.mode = AppMode::NewProjectSetup;
            scale_has_p1 = false;
            scale_has_p2 = false;
        }
        ToolbarTooltip("Recalibrate the floorplan scale.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        ImGui::BeginGroup();
        push_button_group(grey_button, grey_hover, grey_active);
        if (ToolbarButton("Help", false)) {
            show_help_popup = true;
        }
        ToolbarTooltip("Show button descriptions and workflow help.");
        pop_button_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ToolbarGap();

        ImGui::BeginGroup();
        push_menu_group(grey_button, grey_hover, grey_active);
        ImGui::BeginDisabled(!blueprint_finalized);

        if (ImGui::BeginMenu("Save")) {
            // save to computer
            if (ImGui::MenuItem("Save Project")) {
                IGFD::FileDialogConfig config;
                config.path = ".";
                ImGuiFileDialog::Instance()->OpenDialog(
                    "SaveProject",
                    "Save Project",
                    ".slab"
                );
            }
            // export to CSV
            if (ImGui::MenuItem("Export CSV")) {
                IGFD::FileDialogConfig config;
                config.path = default_user_directory();
                config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

                ImGuiFileDialog::Instance()->OpenDialog(
                    "SaveDoseCSV",
                    "Save Dose Results",
                    ".csv",
                    config
                );
            }
            ImGui::EndMenu();
        }
        ToolbarTooltip("Save the project or export results.");
        ImGui::EndDisabled();
        pop_menu_group();
        ImGui::EndGroup();
        draw_cluster_plate(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        draw_list->ChannelsMerge();
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(1);
    }


    void GridRenderer::draw_left_panel() {
        const float panel_width = side_panel_width_px(window);
        ImGui::SetNextWindowPos(ImVec2(0.f, TOOLBAR_HEIGHT_PX));
        ImGui::SetNextWindowSize(ImVec2(panel_width, window.getSize().y - TOOLBAR_HEIGHT_PX));
        ImGui::Begin("Geometric Entity Information", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        
        if (ImGui::BeginTabBar("LeftPanelTabs")) {
            draw_wall_tab();
            draw_source_tab();
            draw_dose_tab();
            draw_isotope_tab();
            ImGui::EndTabBar();
        }
        ImGui::End();
    }


    void GridRenderer::draw_wall_tab() {
        if (!ImGui::BeginTabItem("Wall")) {
            return;
        }

        auto& walls = engine.get_walls_mutable();

        if (walls.empty()) {
            ImGui::TextDisabled("No walls created yet.");
            ImGui::EndTabItem();
            return;
        }

        if (selection.type == Selection::Type::Wall || selection.type == Selection::Type::WallLayer) {
            inspector_wall_index = selection.wall_index;
        }

        if (!inspector_wall_index.has_value()) {
            inspector_wall_index = 0;
        }
        
        std::vector<std::string> wall_labels;
        for (size_t i = 0; i < walls.size(); ++i) {
            wall_labels.push_back("Wall " + std::to_string(i + 1));
        }

        static int wall_idx = 0;
        wall_idx = static_cast<int>(*inspector_wall_index);

        if (ImGui::Combo(
                "Wall",
                &wall_idx,
                [](void* data, int idx) -> const char* {
                    auto& labels = *static_cast<std::vector<std::string>*>(data);
                    return labels[idx].c_str();
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
                    const auto& mat_ids = material_registry.ordered_ids();

                    // Find current index
                    int current_idx = 0;
                    for (int mi = 0; mi < static_cast<int>(mat_ids.size()); ++mi) {
                        if (mat_ids[mi] == layer.material_id) {
                            current_idx = mi;
                            break;
                        }
                    }

                    // Material dropdown
                    if (ImGui::BeginCombo("Material", material_registry.get(mat_ids[current_idx])->name.c_str())) {
                        for (int mi = 0; mi < static_cast<int>(mat_ids.size()); ++mi) {
                            const MaterialDef* m = material_registry.get(mat_ids[mi]);
                            if (!m) continue;

                            bool selected = (mi == current_idx);
                            if (ImGui::Selectable(m->name.c_str(), selected)) {
                                layer.material_id = m->id;
                            }

                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PushTextWrapPos(0.0f);
                    ImGui::TextWrapped("Thickness (cm)");
                    ImGui::PopTextWrapPos();
                    ImGui::InputDouble("##LayerThickness", &layer.thickness_cm);

                    // Remove layer
                    if (wall.layers.size() > 1) {
                        if (ImGui::Button("Remove Layer")) {
                            undo_stack.execute(std::make_unique<RemoveWallLayerCommand>(engine, *inspector_wall_index, i));
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

                undo_stack.execute(std::make_unique<AddWallLayerCommand>(engine, *inspector_wall_index, wall.layers.size() - 1, new_layer));
            }
            ImGui::EndTabBar();
        }
        ImGui::EndTabItem();
    }


    void GridRenderer::draw_source_tab() {
        
        if (!ImGui::BeginTabItem("Source")) {
            return;
        }

        auto& entities = engine.get_entities_mutable();

        std::vector<size_t> sources;
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i].type == PointType::Source) {
                sources.push_back(i);
            }
        }   
        
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

        if (!inspector_source_index.has_value()) {
            inspector_source_index = sources[0];
        }

        // Source selector
        std::vector<std::string> labels;
        for (size_t i = 0; i < sources.size(); ++i) {
            labels.push_back("Source " + std::to_string(i + 1));
        }

        static int src_idx = 0;
        src_idx = std::find(sources.begin(), sources.end(), *inspector_source_index) - sources.begin();

        if (ImGui::Combo(
                "Source",
                &src_idx,
                [](void* data, int idx) -> const char* {
                    auto& labels = *static_cast<std::vector<std::string>*>(data);
                    return labels[idx].c_str();
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

        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("Patients per week");
        ImGui::PopTextWrapPos();
        ImGui::InputFloat("##PatientsPerWeek", &s.num_patients);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("Activity per patient (MBq)");
        ImGui::PopTextWrapPos();
        ImGui::InputFloat("##ActivityPerPatient", &s.activity_per_patient_MBq);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("Uptake time (hrs)");
        ImGui::PopTextWrapPos();
        ImGui::InputFloat("##UptakeTime", &s.uptake_time_hours);

        ImGui::Checkbox("Apply patient attenuation", &s.apply_patient_attenuation);
        if (s.apply_patient_attenuation) {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextWrapped("Patient attenuation [0,1]");
            ImGui::PopTextWrapPos();
            ImGui::InputFloat("##PatientAttenuation", &s.patient_attenuation_percent, 0.05f, 0.1f);
            s.patient_attenuation_percent = std::clamp(s.patient_attenuation_percent, 0.0f, 1.0f);
        }
        ImGui::Checkbox("Apply radioactive decay", &s.apply_radioactive_decay);
        ImGui::EndTabItem();
    }


    void GridRenderer::draw_dose_tab() {
        if (!ImGui::BeginTabItem("Dose")) {
            return;
        }

        auto& entities = engine.get_entities_mutable();

        std::vector<size_t> doses;
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i].type == PointType::Dose) {
                doses.push_back(i);
            }
        }

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

        if (!inspector_dose_index.has_value()) {
            inspector_dose_index = doses[0];
        }

        std::vector<std::string> labels;
        for (size_t i = 0; i < doses.size(); ++i) {
            labels.push_back("Dose " + std::to_string(i + 1));
        }
        
        static int dose_idx = 0;
        dose_idx = std::find(doses.begin(), doses.end(), *inspector_dose_index) - doses.begin();

        if (ImGui::Combo(
                "Dose",
                &dose_idx,
                [](void* data, int idx) -> const char* {
                    auto& labels = *static_cast<std::vector<std::string>*>(data);
                    return labels[idx].c_str();
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
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("Occupancy [0,1]");
        ImGui::PopTextWrapPos();
        ImGui::InputFloat("##Occupancy", &d.occupancy);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("Dose limit (uSv/y)");
        ImGui::PopTextWrapPos();
        ImGui::InputFloat("##DoseLimit", &d.dose_limit_uSv);

        ImGui::EndTabItem();
    }


    void GridRenderer::draw_isotope_tab() {
        if (!ImGui::BeginTabItem("Isotope"))
            return;

        ImGui::Text("Project Isotope");
        ImGui::Separator();
        ImGui::TextWrapped(
            "Select the radioactive isotope used for all sources in this project. "
            "Changing the isotope will affect dose calculations after geometry is locked."
        );
        ImGui::Spacing();

        static std::vector<std::string> isotope_keys;
        static bool initialized = false;

        if (!initialized) {
            isotope_keys.clear();
            for (const std::string& key : isotope_registry.get_all_keys()) {
                isotope_keys.push_back(key);
            }
            initialized = true;
        }

        // Find current index
        int current_idx = 0;
        for (size_t i = 0; i < isotope_keys.size(); ++i) {
            if (isotope_keys[i] == engine.get_selected_isotope_key()) {
                current_idx = i;
                break;
            }
        }

        if (ImGui::Combo(
                "Isotope",
                &current_idx,
                [](void* data, int idx) -> const char* {
                    auto& v = *static_cast<std::vector<std::string>*>(data);
                    return v[idx].c_str();
                },
                &isotope_keys,
                isotope_keys.size()))
        {
            engine.set_selected_isotope_key(isotope_keys[current_idx]);
        }
        ImGui::Spacing();
        ImGui::Separator();

        if (const IsotopeDef* iso = isotope_registry.get_by_key(engine.get_selected_isotope_key())) {
            ImGui::Text("Isotope: %s", iso->name.c_str());
            ImGui::Text("- Gamma constant: %.9f", iso->gamma_constant_uSv_m2_per_MBq_h);
            ImGui::Text("- Half-life: %.2f hours", iso->half_life_hours);
        }
        ImGui::Spacing();
        ImGui::EndTabItem();
    }


    void GridRenderer::handle_events() {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            // Always allow the window to close, regardless of app state.
            if (event->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (app_state.mode == AppMode::ProjectPicker) {
                continue;
            }

            const ImGuiIO& io = ImGui::GetIO();
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

                if (!drawing) { continue; }
                preview_point = p;
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (io.WantCaptureKeyboard) {
                    if (key->code != sf::Keyboard::Key::Left && key->code != sf::Keyboard::Key::Right && key->code != sf::Keyboard::Key::Up && key->code != sf::Keyboard::Key::Down && key->code != sf::Keyboard::Key::Escape && key->code != sf::Keyboard::Key::LShift) {
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
                            .patient_attenuation_percent = 0.0f,    // user defined [0.0, 1.0] patient attenuation
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
        
        // white background behind grid only
        sf::RectangleShape grid_bg;
        const auto& bounds = engine.get_world_bounds();
        grid_bg.setSize(sf::Vector2f(static_cast<float>(bounds.width_cm), static_cast<float>(bounds.height_cm)));
        grid_bg.setPosition(sf::Vector2f{0.f, 0.f});
        grid_bg.setFillColor(Cosmetics::GRID_BACKGROUND);
        window.draw(grid_bg);

        float pixels_per_cm = static_cast<float>(window.getSize().x) / grid_view.getSize().x;
        length_text.setCharacterSize(static_cast<unsigned>(18.f / pixels_per_cm));

        if (background_sprite) {
            window.draw(*background_sprite);
        }

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
            window.clear(Cosmetics::APP_BACKGROUND);
            draw_project_picker();
            return;
        }
        
        // 2. new project setup
        if (app_state.mode == AppMode::NewProjectSetup) {
            window.setView(window.getDefaultView());
            window.clear(Cosmetics::APP_BACKGROUND);
            render_grid_only();    
            draw_new_project_setup();
            return;
        }

        // 3. Editor

        // 4. open project
        if (app_state.mode == AppMode::OpeningProject) {
            window.setView(window.getDefaultView());
            window.clear(Cosmetics::APP_BACKGROUND);

            draw_project_picker();

            if (ImGuiFileDialog::Instance()->Display("OpenProject")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::filesystem::path project_file = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::filesystem::path project_dir = project_file.parent_path();
                    std::string image_path;

                    if (load_project(engine, project_dir.string(), image_path)) {
                        current_floorplan_png_path = image_path;
                        load_background_image(image_path);
                        blueprint_finalized = false;
                        last_compiler_output.reset();
                        selection.clear();
                        drawing = false;
                        update_viewport();
                        app_state.mode = AppMode::Editing;

                        ui_log.push("Project loaded.");
                    } else {
                        ui_log.push("Failed to load project.");
                        app_state.mode = AppMode::ProjectPicker;
                    }
                } else {
                    // user cancelled
                    app_state.mode = AppMode::ProjectPicker;
                }
                
                ImGuiFileDialog::Instance()->Close();
            }

            return;
        }


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

            // hide preview length text if optimization overlay is active
            if (!show_optimization_overlay) {
                double len_cm = distance_cm(start_point, preview_point) * engine.get_distance_scale();
                sf::Vector2f mid{static_cast<float>((start_point.x_cm + preview_point.x_cm) * 0.5), static_cast<float>((start_point.y_cm + preview_point.y_cm) * 0.5)};
                
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << len_cm << " cm";
                length_text.setString(ss.str());
                length_text.setFillColor(Cosmetics::LENGTH_TEXT_COLOR);
                length_text.setOutlineThickness(0.f);
                length_text.setPosition(mid);
                window.draw(length_text);
            }
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

            if (show_optimization_overlay && optimized_scene_cache.has_value()) {
                double old_lead = 0.0;
                double new_lead = 0.0;

                for (const auto& layer : walls[i].layers) {
                    if (const auto* m = material_registry.get(layer.material_id)) {
                        if (m->key == "lead") old_lead = layer.thickness_cm;
                    }
                }

                const auto& opt_wall = optimized_scene_cache->walls[i];
                for (const auto& layer : opt_wall.layers) {
                    if (const auto* m = material_registry.get(layer.material_id)) {
                        if (m->key == "lead") new_lead = layer.thickness_cm;
                    }
                }

                if (std::abs(new_lead - old_lead) > 1e-6) {

                    // highlight wall
                    auto highlight_rect = make_thick_segment(
                        {static_cast<float>(w.a.x_cm), static_cast<float>(w.a.y_cm)},
                        {static_cast<float>(w.b.x_cm), static_cast<float>(w.b.y_cm)},
                        wall_thickness_cm * 1.4f,
                        Cosmetics::WALL_OPTIMIZED
                    );
                    window.draw(highlight_rect);

                    Point mid{(w.a.x_cm + w.b.x_cm) * 0.5, (w.a.y_cm + w.b.y_cm) * 0.5};

                    std::ostringstream ss;
                    ss << std::fixed << std::setprecision(2)
                    << new_lead << " cm Lead";

                    length_text.setString(ss.str());
                    length_text.setFillColor(sf::Color::Black);
                    length_text.setOutlineThickness(2.f);
                    length_text.setOutlineColor(sf::Color::White);
                    length_text.setPosition(sf::Vector2f{static_cast<float>(mid.x_cm), static_cast<float>(mid.y_cm)});

                    window.draw(length_text);
                }
            }

            const Point mid{(w.a.x_cm + w.b.x_cm) * 0.5, (w.a.y_cm + w.b.y_cm) * 0.5};

            if (!show_optimization_overlay && !show_display_info) {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << (w.length_cm * engine.get_distance_scale()) << " cm";
                length_text.setFillColor(Cosmetics::LENGTH_TEXT_COLOR);
                length_text.setOutlineThickness(0.f);
                length_text.setString(ss.str());
                length_text.setPosition(sf::Vector2f{static_cast<float>(mid.x_cm), static_cast<float>(mid.y_cm)});

                window.draw(length_text);
            } else if (!show_optimization_overlay && show_display_info) {
                const std::string wall_label = build_wall_layers_label(w);
                if (!wall_label.empty()) {
                    length_text.setFillColor(Cosmetics::LENGTH_TEXT_COLOR);
                    length_text.setString(wall_label);
                    length_text.setOutlineThickness(1.f);
                    length_text.setOutlineColor(sf::Color::White);
                    length_text.setPosition(sf::Vector2f{static_cast<float>(mid.x_cm), static_cast<float>(mid.y_cm)});
                    window.draw(length_text);
                }
            }
        }

        auto draw_point_label = [&](const std::string& text, const sf::Color& color, const sf::Vector2f& position) {
            if (text.empty()) {
                return;
            }

            length_text.setString(text);
            length_text.setFillColor(color);
            length_text.setOutlineThickness(1.f);
            length_text.setOutlineColor(sf::Color::White);
            length_text.setPosition(position);
            window.draw(length_text);
        };

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

            const sf::Vector2f label_position{
                static_cast<float>(e.position.x_cm + 20),
                static_cast<float>(e.position.y_cm - 30)
            };

            if (show_display_info && e.type == PointType::Source && e.source.has_value()) {
                std::ostringstream ss;
                ss << (e.label.empty() ? "Source" : e.label)
                   << "\n"
                   << fmt_compact(e.source->num_patients, 1) << " pt/wk | "
                   << fmt_compact(e.source->activity_per_patient_MBq, 1) << " MBq";
                draw_point_label(ss.str(), Cosmetics::SOURCE_COLOR, label_position);
            }

            if (show_optimization_overlay && optimized_output_cache.has_value()) {
                
                if (e.type == PointType::Dose) {

                    size_t dose_counter = 0;
                    for (size_t j = 0; j < entities.size(); ++j) {
                        if (entities[j].type == PointType::Dose) {
                            if (j == i) break;
                            dose_counter++;
                        }
                    }

                    if (dose_counter < optimized_output_cache->dose_totals.size()) {
                        const auto& d = optimized_output_cache->dose_totals[dose_counter];
                        const std::string dose_name = d.dose_name.empty()
                            ? ("Dose Point " + std::to_string(dose_counter + 1))
                            : d.dose_name;
                        double annual = d.annual_dose_uSv;
                        double limit = d.dose_limit_uSv;
                        bool violation = (annual > limit + 1e-2);  // equality passes

                        std::ostringstream ss;
                        ss << dose_name
                        << "\n"
                        << std::fixed << std::setprecision(2)
                        << annual << " / " << limit << " uSv";

                        draw_point_label(
                            ss.str(),
                            violation ? Cosmetics::DOSE_LIMIT_FAIL : Cosmetics::DOSE_LIMIT_PASS,
                            label_position
                        );
                    }
                }
            } else if (show_display_info && e.type == PointType::Dose && e.dose.has_value()) {
                std::ostringstream ss;
                ss << (e.label.empty() ? "Dose Point" : e.label)
                   << "\nOcc " << fmt_compact(e.dose->occupancy, 2)
                   << " | Limit " << fmt_compact(e.dose->dose_limit_uSv, 2) << " uSv";
                draw_point_label(ss.str(), Cosmetics::DOSE_COLOR, label_position);
            }
        } 

        // temporary panel view
        window.setView(window.getDefaultView());
        draw_toolbar();
        draw_left_panel();

        const float panel_width = side_panel_width_px(window);
        ImGui::SetNextWindowPos(ImVec2(window.getSize().x - panel_width, TOOLBAR_HEIGHT_PX));
        ImGui::SetNextWindowSize(ImVec2(panel_width, window.getSize().y - TOOLBAR_HEIGHT_PX));
        ImGui::Begin("Terminal", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::BeginChild("terminal_scroll");
        
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
        for (const auto& line : ui_log.lines) {
            ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::PopTextWrapPos();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::End();

        if (ImGuiFileDialog::Instance()->Display("SaveProject")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string project_dir = ImGuiFileDialog::Instance()->GetFilePathName();
                
                if (save_project(engine, project_dir, current_floorplan_png_path)) {
                    ui_log.push("Project saved. You may safely close the application.");
                } else {
                    ui_log.push("Error: Failed to save project.");
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveDoseCSV")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filepath = ImGuiFileDialog::Instance()->GetFilePathName();

                if (last_compiler_output.has_value()) {
                    if (export_compiler_output_csv(*last_compiler_output, filepath)) {
                        ui_log.push("CSV exported to:");
                        ui_log.push(filepath);
                    } else {
                        ui_log.push("CSV export failed.");
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (show_isotope_popup) {
            ImGui::OpenPopup("Isotope Library");
            show_isotope_popup = false;
        }

        ImGui::SetNextWindowSize(ImVec2(1100, 650), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("Isotope Library", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::Text("Isotopes");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("IsoScrollRegion", ImVec2(0, -40), false);
            int columns = 3;
            ImGui::Columns(columns, nullptr, false);

            for (const std::string& key : isotope_registry.get_all_keys()) {

                if (const auto* iso = isotope_registry.get_by_key(key)) {
                    ImGui::BeginChild(key.c_str(), ImVec2(0, 0), true);
                    ImGui::Text("%s", iso->name.c_str());
                    ImGui::Separator();
                    ImGui::BulletText("Gamma Constant: %.6e uSv_m^2/MBq_h", iso->gamma_constant_uSv_m2_per_MBq_h);
                    ImGui::BulletText("Half-life: %.3f hours", iso->half_life_hours);
                    ImGui::Spacing();
                    ImGui::Text("Shielding Data (mm)");
                    ImGui::Separator();

                    // iterate materials for this isotope
                    for (const auto& [mat_key, data] : iso->materials) {
                        ImGui::Text("%s", mat_key.c_str());
                        ImGui::BulletText("HVL1_mm: %.2f", data.hvl1_mm);
                        ImGui::BulletText("HVL2_mm: %.2f", data.hvl2_mm);
                        ImGui::BulletText("TVL1_mm: %.2f", data.tvl1_mm);
                        ImGui::BulletText("TVL2_mm: %.2f", data.tvl2_mm);
                        ImGui::Spacing();
                    }
                    ImGui::EndChild();
                }
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::Spacing();

            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (show_material_popup) {
            ImGui::OpenPopup("Material Library");
            show_material_popup = false;
        }

        if (ImGui::BeginPopupModal("Material Library", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Materials");
            ImGui::Separator();

            for (int id : material_registry.ordered_ids()) {
                if (const auto* mat = material_registry.get(id)) {
                    ImGui::Text("%s", mat->name.c_str());
                    ImGui::BulletText("Key: %s", mat->key.c_str());
                    ImGui::Spacing();
                }
            }

            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (show_help_popup) {
            ImGui::OpenPopup("Help");
            show_help_popup = false;
        }

        ImGui::SetNextWindowSize(ImVec2(760, 520), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Help", nullptr, ImGuiWindowFlags_NoCollapse)) {
            ImGui::Spacing();

            ImGui::BeginChild("HelpScrollRegion", ImVec2(0, -40), true);
            const auto help_row = [](const char* label, const char* description) {
                DrawHelpButtonPreview(label);
                ImGui::SameLine();
                ImGui::TextWrapped("%s", description);
                ImGui::Spacing();
            };

            help_row("Selection", "Select and inspect geometry.");
            help_row("Editing", "Switch to placement and editing mode.");
            help_row("Add Wall", "Place a wall segment.");
            help_row("Add Source", "Place a radiation source.");
            help_row("Add Dose", "Place a dose point.");
            help_row("Delete", "Remove the selected item.");
            help_row("Undo", "Reverse the last action.");
            help_row("Redo", "Restore the last undone action.");
            help_row("Isotopes", "Show the list of supported isotopes.");
            help_row("Materials", "Show the list of supported materials.");
            help_row("Run Calc", "Calculate dose and lock geometry.");
            help_row("Unlock Geometry", "Unlock geometry so you can edit again.");
            help_row("Optimize", "Optimize shielding.");
            help_row("Results", "Show optimization results.");
            help_row("Info", "Toggle wall, source, and dose annotations.");
            help_row("Edit Scale", "Recalibrate the floorplan scale.");
            help_row("Help", "Open this help window.");
            help_row("Save", "Save the project or export results.");

            ImGui::TextWrapped("Typical workflow: Upload the PDF floorplan, calibrate the real-world scale, add walls, sources, and dose points, adjust wall materials and thicknesses, enter the source and dose-point data, review the supported isotopes and materials, select the isotope to use throughout the calculation, run the initial calculation, and then optimize and review the results if needed.");
            ImGui::EndChild();

            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    
    }

} // end namespace geom
