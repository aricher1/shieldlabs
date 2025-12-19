#include "ui/GridRenderer.hpp"
#include <algorithm>
#include <cmath>



GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e) : window(w), engine(e) {
    const auto size = window.getSize();
    window_size = size;

    origin_px = {
        static_cast<float>(size.x) / 2.0f,
        static_cast<float>(size.y) / 2.0f
    };
}

Point GridRenderer::screen_to_world(sf::Vector2i mouse) const {
    
    return {
        (mouse.x - origin_px.x) / pixels_per_cm,
        (origin_px.y - mouse.y) / pixels_per_cm
    };
}


sf::Vector2f GridRenderer::world_to_screen(Point p) const {

    return {
        static_cast<float>(p.x_cm * pixels_per_cm + origin_px.x),
        static_cast<float>(origin_px.y - p.y_cm * pixels_per_cm)
    };
}


Point GridRenderer::snap_to_grid(Point p) const {
    const double spacing = engine.get_grid_spacing_cm();

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

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                
                sf::Vector2i mouse_px = sf::Mouse::getPosition(window);
                sf::Vector2f coords = window.mapPixelToCoords(mouse_px);

                Point p = snap_to_grid(
                    screen_to_world({
                        static_cast<int>(coords.x),
                        static_cast<int>(coords.y)
                    })
                );

                if (!drawing) {
                    start_point = p;
                    drawing = true;
                } else {
                    engine.add_wall(start_point, p, 20.0, 1.0);
                    drawing = false;
                }
            }
        }
    }
}


void GridRenderer::render() {
    window.clear(sf::Color(255, 255, 255));

    // draw grid
    const double grid_spacing_cm = engine.get_grid_spacing_cm();

    const Point top_left = screen_to_world({0, 0});
    const sf::Vector2u size = window.getSize();
    const Point bottom_right = screen_to_world({static_cast<int>(size.x), static_cast<int>(size.y)});

    const double min_x = std::min(top_left.x_cm, bottom_right.x_cm);
    const double max_x = std::max(top_left.x_cm, bottom_right.x_cm);
    const double min_y = std::min(top_left.y_cm, bottom_right.y_cm);
    const double max_y = std::max(top_left.y_cm, bottom_right.y_cm);

    const double start_x = std::floor(min_x / grid_spacing_cm) * grid_spacing_cm;
    const double start_y = std::floor(min_y / grid_spacing_cm) * grid_spacing_cm;

    for (double x = start_x; x <= max_x; x += grid_spacing_cm) {

        sf::Vertex vline[2];
        vline[0].position = world_to_screen({x, min_y});
        vline[0].color = sf::Color(220, 220, 220);

        vline[1].position = world_to_screen({x, max_y});
        vline[1].color = sf::Color(220, 220, 220);

        window.draw(vline, 2, sf::PrimitiveType::Lines);
    }

    for (double y = start_y; y <= max_y; y += grid_spacing_cm) {

        sf::Vertex hline[2];
        hline[0].position = world_to_screen({min_x, y});
        hline[0].color = sf::Color(220, 220, 220);

        hline[1].position = world_to_screen({max_x, y});
        hline[1].color = sf::Color(220, 220, 220);

        window.draw(hline, 2, sf::PrimitiveType::Lines);
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

    window.display();
}