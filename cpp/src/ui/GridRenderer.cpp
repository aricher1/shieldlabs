#include "ui/GridRenderer.hpp"
#include "ui/AddWallCommand.hpp"
#include "ui/DeleteWallCommand.hpp"
#include "ui/RemoveEntityCommand.hpp"
#include "ui/AddEntityCommand.hpp"
#include "ui/AddOpeningCommand.hpp"
#include "ui/RemoveOpeningCommand.hpp"
#include "ui/ShiftGeometryCommand.hpp"
#include "ui/Cosmetics.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>



namespace {

    constexpr double SELECT_EPS_CM = 25.0; // select wall/point by clicking within 25cm of it
    constexpr double SNAP_POINT_EPS_CM = 0.01;  // snap epsilon, checking if p equals an existing endpoint for selection logic
    constexpr float PICK_RADIUS_PX = 10.0f;
    constexpr double SHIFT_MOVE_MULTIPLIER = 10.0; // when shift is pressed, the drawing shifts SHIFT_MOVE_MULTIPLIER times the normal distance

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


} // end of anonymous namespace


GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e) : window(w), engine(e), length_text(font) {
    
    window_size = window.getSize();
    const float world_size = static_cast<float>(engine.get_grid_cells() * engine.get_cm_per_cell());
    grid_view.setSize(sf::Vector2f{world_size, world_size});
    grid_view.setCenter(sf::Vector2f{0.f, 0.f});
    float win_w = static_cast<float>(window_size.x);
    float win_h = static_cast<float>(window_size.y);
    float scale = std::min(win_w, win_h);
    grid_view.setViewport(sf::FloatRect{{(win_w - scale) / win_w / 2.f, (win_h - scale) / win_h / 2.f}, {scale / win_w, scale / win_h}});
    if (!font.openFromFile("assets/fonts/Inter-Regular.otf")) {
        std::cerr << "Failed to load font\n" << std::endl;
    }
    length_text.setFont(font);
    length_text.setCharacterSize(14);
    length_text.setFillColor(sf::Color::Black);
}


double GridRenderer::pixel_radius_to_world_cm(float px) const {
    sf::Vector2f p0 = window.mapPixelToCoords({0, 0}, grid_view);
    sf::Vector2f p1 = window.mapPixelToCoords({(int)px, 0}, grid_view);
    
    return std::abs(p1.x - p0.x);
}


Point GridRenderer::screen_to_world(sf::Vector2f mouse) const {
    const double world_size_cm = engine.get_grid_cells() * engine.get_cm_per_cell();
    const double nx = mouse.x / grid_view.getSize().x;
    const double ny = mouse.y / grid_view.getSize().y;

    return {(nx - 0.5) * world_size_cm, (0.5 - ny) * world_size_cm};
}


void GridRenderer::finalize_blueprint() {
    // error handling
    if (!blueprint_finalized) {
        auto errors = engine.validate();
        if (!errors.empty()) {
            std::cerr << "VALIDATION ERRORS:\n";
            for (const auto& e : errors) {
                std::cerr << e.message << "\n";
            }
            return;
        }
        std::cerr << "Validation passed.\n";
    }

    // toggle finalized state
    blueprint_finalized = !blueprint_finalized;

    // cancel any current edits
    drawing = false;
    placing_opening = false;
    selection.clear();

    if (blueprint_finalized) {
        std::cout << "========= Final Blueprint =========\n";
        std::cout << engine.to_json() << "\n";
    } else {
        std::cout << "========= Editing Resumed =========\n";
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


void GridRenderer::handle_events() {
    while (const auto event = window.pollEvent()) {
        
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            window_size = {resized->size.x, resized->size.y};
            float win_w = static_cast<float>(window_size.x);
            float win_h = static_cast<float>(window_size.y);
            float scale = std::min(win_w, win_h);
            grid_view.setViewport(sf::FloatRect{{(win_w - scale) / win_w / 2.0f, (win_h - scale) / win_h / 2.0f}, {scale / win_w, scale / win_h}});

        }

        if (const auto* move = event->getIf<sf::Event::MouseMoved>()) {
            
            sf::Vector2i mouse_px {move->position.x, move->position.y};
            sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);
            Point p = engine.snap_to_grid({mouse_world.x, mouse_world.y});

            if (placing_opening) {

                const Wall& w = engine.get_walls()[opening_wall_index];
                double t = project_t_onto_wall(p, w);
                preview_point = lerp_point(w.a, w.b, t);
                continue;

            }

            if (!drawing) {
                continue;
            }

            
            preview_point = p;

        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            
            /* 
            ======== Controls ========
            - Cmd + Z = undo
            - Cmd + Shift + Z = redo
            - Click near wall -> turns green -> delete or backspace = remove
            - Esc = cancel drawing
            - W = draw wall
            - S = place source point
            - D = place dose point
            - Space = switch between draw and select mode
            - F = finalize blueprint
            - O = draw opening
            - L = draw door
            - M = draw window
            */


            /* 
            To Do
            - material selection logic
            - refactor JSON to a cleaner version for calculation ready files
            */

            if (key->code == sf::Keyboard::Key::F) {
                finalize_blueprint();
            }

            if (key->code == sf::Keyboard::Key::Space) {
                
                interaction_mode = (interaction_mode == InteractionMode::Draw) ? InteractionMode::Select : InteractionMode::Draw;
                // cancel all active interactions
                selection.clear();
                drawing = false;
                placing_opening = false;
            }

            if (key->code == sf::Keyboard::Key::Z && key->system) {

                if (key->shift) {
                    undo_stack.redo();
                } else {
                    undo_stack.undo();
                }
            }

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

            if (key->code == sf::Keyboard::Key::Delete || key->code == sf::Keyboard::Key::Backspace) {
                
                switch (selection.type) {
                    
                    case Selection::Type::Opening:
                        undo_stack.execute(std::make_unique<RemoveOpeningCommand>(engine, selection.wall_index, selection.opening_index));
                        selection.clear();
                        return;
                    
                    case Selection::Type::Entity:
                        undo_stack.execute(std::make_unique<RemoveEntityCommand>(engine, selection.entity_index));
                        selection.clear();
                        return;

                    case Selection::Type::Wall:
                        undo_stack.execute(std::make_unique<DeleteWallCommand>(engine, selection.wall_index));
                        selection.clear();
                        return;
                    
                    default:
                        break;
                }
            }

            if (key->code == sf::Keyboard::Key::Escape) {

                if (drawing) {
                    drawing = false; // cancel drawing
                    preview_point = start_point;
                }
            }

            if (key->code == sf::Keyboard::Key::W) {
                current_tool = Tool::DrawWall;
            }

            if (key->code == sf::Keyboard::Key::S) {
                current_tool = Tool::PlaceSource;
            }

            if (key->code == sf::Keyboard::Key::D) {
                current_tool = Tool::PlaceDose;
            }

            if (key->code == sf::Keyboard::Key::O) {
                current_tool = Tool::PlaceOpen;
            }

            if (key->code == sf::Keyboard::Key::L) {
                current_tool = Tool::PlaceDoor;
            }

            if (key->code == sf::Keyboard::Key::M) {
                current_tool = Tool::PlaceWindow;
            }

        }

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            
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
                            o.type = (current_tool == Tool::PlaceDoor) ? OpeningType::Door : (current_tool == Tool::PlaceWindow) ? OpeningType::Window : OpeningType::Open;
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
                    opening_type = (current_tool == Tool::PlaceDoor) ? OpeningType::Door : (current_tool == Tool::PlaceWindow) ? OpeningType::Window : OpeningType::Open;
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
                    undo_stack.execute(std::make_unique<AddEntityCommand>(engine, e));
                    return;
                }

                if (current_tool == Tool::PlaceDose) {
                    PointEntity e;
                    e.position = p;
                    e.type = PointType::Dose;
                    e.label = "";
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
                    std::cout << "A: " << start_point.x_cm << ", " << start_point.y_cm << "  B: " << p.x_cm << ", " << p.y_cm << "\n";
                    Wall wall;
                    wall.a = start_point;
                    wall.b = p;
                    wall.thickness_cm = 20.0;
                    wall.material_id = 1;
                    wall.length_cm = std::hypot(wall.a.x_cm - wall.b.x_cm, wall.a.y_cm - wall.b.y_cm);
                    undo_stack.execute(std::make_unique<AddWallCommand>(engine, wall));
                    drawing = false;
                }
            }
        }
    }
}


void GridRenderer::render() {
    window.clear(sf::Color::White);
    window.setView(grid_view);

    // draw grid
    const double half = (engine.get_grid_cells() * engine.get_cm_per_cell()) / 2.0;
    const double min_x = -half;
    const double max_x = half;
    const double min_y = -half;
    const double max_y = half;
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
        sf::Vertex preview[2];
        preview[0].position = sf::Vector2f{static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)};
        preview[1].position = sf::Vector2f{static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)};
        preview[0].color = Cosmetics::WALL_NORMAL;
        preview[1].color = Cosmetics::WALL_NORMAL;        
        window.draw(preview, 2, sf::PrimitiveType::Lines);
        double len_cm = distance_cm(start_point, preview_point);
        sf::Vector2f mid{static_cast<float>((start_point.x_cm + preview_point.x_cm) / 2.0), static_cast<float>((start_point.y_cm + preview_point.y_cm) / 2.0)};
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
        sf::Vertex wall[2];
        wall[0].position = sf::Vector2f{static_cast<float>(w.a.x_cm), static_cast<float>(w.a.y_cm)};
        wall[1].position = sf::Vector2f{static_cast<float>(w.b.x_cm), static_cast<float>(w.b.y_cm)};
        if (selection.type == Selection::Type::Wall && selection.wall_index == i) {
            wall[0].color = sf::Color::Green;
            wall[1].color = sf::Color::Green;
        } else {
            wall[0].color = sf::Color::Black;
            wall[1].color = sf::Color::Black;
        }

        window.draw(wall, 2, sf::PrimitiveType::Lines);
        
        if (placing_opening && i == opening_wall_index) {
            sf::Vertex preview[2];
            preview[0].position = sf::Vector2f{static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)};
            preview[1].position = sf::Vector2f{static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)};
            sf::Color c;
            switch (opening_type) {
                case OpeningType::Door: c = Cosmetics::DOOR_COLOR; break;
                case OpeningType::Window: c = Cosmetics::WINDOW_COLOR; break;
                case OpeningType::Open: c = Cosmetics::OPEN_COLOR; break;
            }
            auto rect = make_thick_segment({static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)}, {static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)}, Cosmetics::OPENING_THICKNESS, c);
            window.draw(rect);

            // draw text
            double len_cm = distance_cm(start_point, preview_point);
            sf::Vector2f mid{
                static_cast<float>((start_point.x_cm + preview_point.x_cm) * 0.5), 
                static_cast<float>((start_point.y_cm + preview_point.y_cm) * 0.5)
            };

            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << len_cm << " cm";
            length_text.setString(ss.str());

            // color matches opening type on top of wall
            switch (opening_type) {
                case OpeningType::Door: length_text.setFillColor(Cosmetics::DOOR_TEXT_COLOR); break;
                case OpeningType::Window: length_text.setFillColor(Cosmetics::WINDOW_TEXT_COLOR); break;
                case OpeningType::Open: length_text.setFillColor(Cosmetics::OPEN_TEXT_COLOR); break;
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
                    case OpeningType::Door: c = Cosmetics::DOOR_COLOR; break;
                    case OpeningType::Window: c = Cosmetics::WINDOW_COLOR; break;
                    case OpeningType::Open: c = Cosmetics::OPEN_COLOR; break;
                }
            }

            auto rect = make_thick_segment({static_cast<float>(p0.x_cm), static_cast<float>(p0.y_cm)}, {static_cast<float>(p1.x_cm), static_cast<float>(p1.y_cm)}, Cosmetics::OPENING_THICKNESS, c);

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
            sf::Vector2f text_position = mid + normal * (Cosmetics::OPENING_THICKNESS * 1.0f);

            // actual text
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << o.length_cm << " cm";
            length_text.setString(ss.str());

            switch (o.type) {
                case OpeningType::Door: length_text.setFillColor(Cosmetics::DOOR_TEXT_COLOR); break;
                case OpeningType::Window: length_text.setFillColor(Cosmetics::WINDOW_TEXT_COLOR); break;
                case OpeningType::Open: length_text.setFillColor(Cosmetics::OPEN_TEXT_COLOR); break;
            }

            if (selection.type == Selection::Type::Opening && selection.wall_index == i && selection.opening_index == (&o - &w.openings[0])) {
                length_text.setFillColor(Cosmetics::WALL_SELECTED);
            }

            length_text.setPosition(text_position);
            window.draw(length_text);
        }

        const Point mid{(w.a.x_cm + w.b.x_cm) * 0.5, (w.a.y_cm + w.b.y_cm) * 0.5};

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << w.length_cm << " cm";

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
        marker.setRadius(Cosmetics::POINT_RADIUS);
        marker.setOrigin(sf::Vector2f{Cosmetics::POINT_RADIUS, Cosmetics::POINT_RADIUS});
        marker.setPosition(sf::Vector2f{static_cast<float>(e.position.x_cm), static_cast<float>(e.position.y_cm)});

        const bool selected = selection.type == Selection::Type::Entity && selection.entity_index == i;

        if (e.type == PointType::Source) {

            marker.setFillColor(selected ? Cosmetics::SOURCE_SELECTED_COLOR : Cosmetics::SOURCE_COLOR);

        } else {
                
            marker.setFillColor(sf::Color::Transparent);
            marker.setOutlineThickness(Cosmetics::POINT_OUTLINE_THICKNESS);
            marker.setOutlineColor(selected ? Cosmetics::DOSE_SELECTED_COLOR : Cosmetics::DOSE_COLOR);

        }

        window.draw(marker);
    } 

    window.setView(window.getDefaultView());
    window.display();
}