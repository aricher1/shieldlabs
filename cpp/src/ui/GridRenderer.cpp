#include "ui/GridRenderer.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>


static constexpr int GRID_CELLS = 100;
static constexpr double CM_PER_CELL = 10.0;



GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e) : window(w), engine(e) {
    
    window_size = window.getSize();

    float size = std::min(
        static_cast<float>(window_size.x),
        static_cast<float>(window_size.y)
    );

    grid_viewport = sf::FloatRect{{(window_size.x - size) / 2.0f, (window_size.y - size) / 2.0f}, {size, size}};

    grid_view.setSize(sf::Vector2f{size, size});
    grid_view.setCenter(sf::Vector2f{size / 2.0f, size / 2.0f});

    grid_view.setViewport(sf::FloatRect{{grid_viewport.position.x / window_size.x, grid_viewport.position.y / window_size.y}, {grid_viewport.size.x / window_size.x, grid_viewport.size.y / window_size.y}});

}


Point GridRenderer::screen_to_world(sf::Vector2f mouse) const {
    
    const double world_size_cm = GRID_CELLS * CM_PER_CELL;
    
    const double nx = mouse.x / grid_view.getSize().x;
    const double ny = mouse.y / grid_view.getSize().y;

    /*
    const double nx = (mouse.x - grid_viewport.position.x) / grid_viewport.size.x;
    const double ny = (mouse.y - grid_viewport.position.y) / grid_viewport.size.y;
    */

    return {

        (nx - 0.5) * world_size_cm, (0.5 - ny) * world_size_cm
    
    };
}


sf::Vector2f GridRenderer::world_to_screen(Point p) const {

    const double world_size_cm = GRID_CELLS * CM_PER_CELL;
    const double pixels_per_cm = grid_view.getSize().x / world_size_cm;

    // const double pixels_per_cm = grid_viewport.size.x / world_size_cm;

    return {
        
        grid_viewport.position.x + static_cast<float>((p.x_cm + world_size_cm / 2.0) * pixels_per_cm),

        grid_viewport.position.y + static_cast<float>((world_size_cm / 2.0 - p.y_cm) * pixels_per_cm)

    };
}


Point GridRenderer::snap_to_grid(Point p) const {
    
    const double spacing = cm_per_cell;

    return {

        std::round(p.x_cm / spacing) * spacing,
        std::round(p.y_cm / spacing) * spacing
    
    };
}


void GridRenderer::handle_events() {
    while (const auto event = window.pollEvent()) {
        
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            
            window_size = {resized->size.x, resized->size.y};
            
            float size = std::min(static_cast<float>(window_size.x), static_cast<float>(window_size.y));

            grid_viewport = {{(window_size.x - size) / 2.0f, (window_size.y - size) / 2.0f}, {size, size}};

        }
/*
            const Point center_world = screen_to_world({
                static_cast<int>(window_size.x / 2),
                static_cast<int>(window_size.y / 2)
            });

            const sf::Vector2u new_size = {resized->size.x, resized->size.y};
            const float width_ratio = static_cast<float>(new_size.x) / static_cast<float>(window_size.x);

            pixels_per_cm = std::clamp(
                pixels_per_cm * width_ratio,
                MIN_PIXELS_PER_CM,
                MAX_PIXELS_PER_CM
            );

            origin_px = {
                static_cast<float>(new_size.x) / 2.0f - static_cast<float>(center_world.x_cm * pixels_per_cm),
                static_cast<float>(new_size.y) / 2.0f + static_cast<float>(center_world.y_cm * pixels_per_cm)
            };

            window_size = new_size;
        }

        if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
            const float zoom_factor = wheel->delta > 0 ? 1.1f : 1.0f / 1.1f;
            pixels_per_cm = std::clamp(
                pixels_per_cm * zoom_factor,
                MIN_PIXELS_PER_CM,
                MAX_PIXELS_PER_CM
            );
        }
*/
        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                
                sf::Vector2i mouse_px{static_cast<int>(mouse->position.x), static_cast<int>(mouse->position.y)};

                sf::Vector2f mouse_coords = window.mapPixelToCoords(mouse_px);

                // sf::Vector2i mouse_px = sf::Mouse::getPosition(window);
                // sf::Vector2f coords = window.mapPixelToCoords(mouse_px);

                if (!grid_viewport.contains(sf::Vector2f{static_cast<float>(mouse_px.x), static_cast<float>(mouse_px.y)})) {
                    continue;
                }

                Point p = snap_to_grid(
                    screen_to_world(mouse_coords)
                    //screen_to_world({
                    //    static_cast<int>(coords.x),
                    //    static_cast<int>(coords.y)
                    //})
                );

                if (!drawing) {
                    start_point = p;
                    drawing = true;
                } else {
                    std::cout << "A: " << start_point.x_cm << ", " << start_point.y_cm
                              << "  B: " << p.x_cm << ", " << p.y_cm << "\n";
                    engine.add_wall(start_point, p, 20.0, 1.0);
                    drawing = false;
                }
            }
        }
    }
}


void GridRenderer::render() {
    window.clear(sf::Color(255, 255, 255));

    // window.setView(grid_view);

    // draw grid
    const double half = (GRID_CELLS * CM_PER_CELL) / 2.0;

    const double min_x = -half;
    const double max_x = half;
    const double min_y = -half;
    const double max_y = half;

    const double grid_spacing_cm = CM_PER_CELL;

/*
    const Point top_left = screen_to_world({0, 0});
    const sf::Vector2u size = window.getSize();
    const Point bottom_right = screen_to_world({static_cast<int>(size.x), static_cast<int>(size.y)});

    const double min_x = std::min(top_left.x_cm, bottom_right.x_cm);
    const double max_x = std::max(top_left.x_cm, bottom_right.x_cm);
    const double min_y = std::min(top_left.y_cm, bottom_right.y_cm);
    const double max_y = std::max(top_left.y_cm, bottom_right.y_cm);

    const double start_x = std::floor(min_x / grid_spacing_cm) * grid_spacing_cm;
    const double start_y = std::floor(min_y / grid_spacing_cm) * grid_spacing_cm;
*/

    for (double x = min_x; x <= max_x; x += grid_spacing_cm) {

        sf::Vertex line[2];
        
        line[0].position = world_to_screen({x, min_y});
        line[0].color = sf::Color(220, 220, 220);

        line[1].position = world_to_screen({x, max_y});
        line[1].color = sf::Color(220, 220, 220);

        window.draw(line, 2, sf::PrimitiveType::Lines);

    }

    for (double y = min_y; y <= max_y; y += grid_spacing_cm) {

        sf::Vertex line[2];
        
        line[0].position = world_to_screen({min_x, y});
        line[0].color = sf::Color(220, 220, 220);

        line[1].position = world_to_screen({max_x, y});
        line[1].color = sf::Color(220, 220, 220);

        window.draw(line, 2, sf::PrimitiveType::Lines);

    }

    // draw walls
    for (const auto& w : engine.get_walls()) {
        
        sf::Vertex wall[2];

        wall[0].position = world_to_screen(w.a);
        wall[0].color = sf::Color(0, 0, 255);

        wall[1].position = world_to_screen(w.b);
        wall[1].color = sf::Color(0, 0, 255);

        window.draw(wall, 2, sf::PrimitiveType::Lines);
    }

    // window.setView(window.getDefaultView());

    window.display();
}