#include "ui/GridRenderer.hpp"
#include "ui/AddWallCommand.hpp"
#include "ui/DeleteWallCommand.hpp"
#include "ui/RemoveEntityCommand.hpp"
#include "ui/AddEntityCommand.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>



namespace {

    constexpr double SELECT_EPS_CM = 25.0; // select wall/point by clicking within 25cm of it
    constexpr double SNAP_POINT_EPS_CM = 0.01;  // snap epsilon, checking if p equals an existing endpoint for selection logic

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


Point GridRenderer::screen_to_world(sf::Vector2f mouse) const {
    
    const double world_size_cm = engine.get_grid_cells() * engine.get_cm_per_cell();
    
    const double nx = mouse.x / grid_view.getSize().x;
    const double ny = mouse.y / grid_view.getSize().y;

    return {

        (nx - 0.5) * world_size_cm, (0.5 - ny) * world_size_cm
    
    };
}


Point GridRenderer::snap_to_grid(Point p) const {
    
    const double spacing = cm_per_cell;

    return {

        std::round(p.x_cm / spacing) * spacing,
        std::round(p.y_cm / spacing) * spacing
    
    };
}


void GridRenderer::finalize_blueprint() {

    if (blueprint_finalized) { return; } // default value is false

    blueprint_finalized = true;

    // cancel any current edits
    drawing = false;
    selected_wall_index.reset();
    selected_entity_index.reset();

    // print to console for now
    std::cout << "========= Final Blueprint =========\n";
    std::cout << engine.to_json() << "\n";

}


void GridRenderer::handle_select_click(const Point& p) {

    selected_entity_index.reset();
    selected_wall_index.reset();

    // entity selection
    const auto& entities = engine.get_entities();
    double best_entity_dist = SELECT_EPS_CM;

    for (std::size_t i = 0; i < entities.size(); ++i) {

        double d = distance_point_to_point(p, entities[i].position);

        if (d < best_entity_dist) {

            best_entity_dist = d;
            selected_entity_index = i;
        
        }
    }

    if (selected_entity_index.has_value()) { return ; }

    // wall selection
    const auto& walls = engine.get_walls();
    double best_wall_dist = SELECT_EPS_CM;

    for (std::size_t i = 0; i < walls.size(); ++i) {

        double d = distance_point_to_segment(p, walls[i].a, walls[i].b);

        if (d < best_wall_dist) {

            best_wall_dist = d;
            selected_wall_index = i;

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
            
            if (!drawing) {
                continue;
            }

            sf::Vector2i mouse_px {move->position.x, move->position.y};

            sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);

            preview_point = snap_to_grid({mouse_world.x, mouse_world.y});

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
            */

            if (key->code == sf::Keyboard::Key::F) {

                finalize_blueprint();

            }

            if (key->code == sf::Keyboard::Key::Space) {
                
                interaction_mode = (interaction_mode == InteractionMode::Draw) ? InteractionMode::Select : InteractionMode::Draw;

                // clear selection when switching modes
                selected_wall_index.reset();
                selected_entity_index.reset();
                drawing = false;

            }

            if (key->code == sf::Keyboard::Key::Z && key->system) {

                if (key->shift) {

                    undo_stack.redo();

                } else {

                    undo_stack.undo();

                }
            }

            if (key->code == sf::Keyboard::Key::Delete || key->code == sf::Keyboard::Key::Backspace) {

                // Priority 1: point entities
                if (selected_entity_index.has_value()) {

                    undo_stack.execute(std::make_unique<RemoveEntityCommand>(engine, *selected_entity_index));
                    selected_entity_index.reset();
                    selected_wall_index.reset();

                }   

                // Priority 2: walls         
                else if (selected_wall_index.has_value()) {

                    undo_stack.execute(std::make_unique<DeleteWallCommand>(engine, *selected_wall_index));
                    selected_wall_index.reset();

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

        }

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            
            if (mouse->button == sf::Mouse::Button::Left) {

                // stop editing once finalized                
                if (blueprint_finalized) { return; }

                sf::Vector2i mouse_px{mouse->position.x, mouse->position.y};
                sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);
                Point p = snap_to_grid({mouse_world.x, mouse_world.y});

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

        line[0].color = sf::Color(220, 220, 220);
        line[1].color = sf::Color(220, 220, 220);

        window.draw(line, 2, sf::PrimitiveType::Lines);

    }

    for (double y = min_y; y <= max_y; y += grid_spacing_cm) {

        sf::Vertex line[2];
        
        line[0].position = sf::Vector2f{static_cast<float>(min_x), static_cast<float>(y)};
        line[1].position = sf::Vector2f{static_cast<float>(max_x), static_cast<float>(y)};

        line[0].color = sf::Color(220, 220, 220);
        line[1].color = sf::Color(220, 220, 220);

        window.draw(line, 2, sf::PrimitiveType::Lines);

    }

    if (drawing) {

        sf::Vertex preview[2];

        preview[0].position = sf::Vector2f{static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)};
        preview[1].position = sf::Vector2f{static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)};

        preview[0].color = sf::Color(0, 0, 0, 120);
        preview[1].color = sf::Color(0, 0, 0, 120);

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

        if (selected_wall_index && *selected_wall_index == i) {

            wall[0].color = sf::Color::Red;
            wall[1].color = sf::Color::Red;

        } else {

            wall[0].color = sf::Color::Black;
            wall[1].color = sf::Color::Black;

        }

        window.draw(wall, 2, sf::PrimitiveType::Lines);

        const Point mid{(w.a.x_cm + w.b.x_cm) * 0.5, (w.a.y_cm + w.b.y_cm) * 0.5};

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << w.length_cm << " cm";

        length_text.setString(ss.str());
        length_text.setPosition(sf::Vector2f{static_cast<float>(mid.x_cm), static_cast<float>(mid.y_cm)});

        window.draw(length_text);
    }

    // draw source + dose points
    const auto& entities = engine.get_entities();
    for (std::size_t i = 0; i < entities.size(); ++i) {

        const auto& e = entities[i];

        sf::CircleShape marker;
        marker.setRadius(5.0f);
        marker.setOrigin(sf::Vector2f{5.0f, 5.0f});
        marker.setPosition(sf::Vector2f{static_cast<float>(e.position.x_cm), static_cast<float>(e.position.y_cm)});

        const bool selected = selected_entity_index.has_value() && *selected_entity_index == i;

        if (e.type == PointType::Source) {

            marker.setFillColor(selected ? sf::Color::Red : sf::Color::Green);

        } else {
                
            marker.setFillColor(sf::Color::Transparent);
            marker.setOutlineThickness(1.5f);
            marker.setOutlineColor(selected ? sf::Color::Red : sf::Color::Blue);

        }

        window.draw(marker);
    } 

    window.setView(window.getDefaultView());
    window.display();
}