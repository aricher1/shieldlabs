#include "ui/GridRenderer.hpp"
#include <cmath>



GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e) : window(w), engine(e) {} 

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


void GridRenderer::handle_events() {
    while (const auto event = window.pollEvent()) {
        
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left) {
                Point p = screen_to_world(mouse->position);

                /*
                Point p = screen_to_world({
                    static_cast<float>(mouse->position.x),
                    static_cast<float>(mouse->position.y)
                });
                */

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
    const int grid_spacing_cm = 50;

    for (int i = -1000; i <= 1000; i+= grid_spacing_cm) {
        
        sf::Vertex vline[2];
        vline[0].position = world_to_screen({(double)i, -1000});
        vline[0].color = sf::Color(220, 220, 220);

        vline[1].position = world_to_screen({(double)i, 1000});
        vline[1].color = sf::Color(220, 220, 220);
        
        window.draw(vline, 2, sf::PrimitiveType::Lines);

        sf::Vertex hline[2];
        hline[0].position = world_to_screen({-1000.0, static_cast<double>(i)});
        hline[0].color = sf::Color(220, 220, 220);

        hline[1].position = world_to_screen({1000.0, static_cast<double>(i)});
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