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
        p.x_cm * pixels_per_cm + origin_px.x,
        origin_px.y - p.y_cm * pixels_per_cm
    };
}


void GridRenderer::handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

            Point p = screen_to_world({event.mouseButton.x, event.mouseButton.y});

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


void GridRenderer::render() {
    window.clear(sf::Color::White);

    // draw grid
    const int grid_spacing_cm = 50;

    for (int i = -1000; i <= 1000; i+= grid_spacing_cm) {
        
        sf::Vertex vline[] = {
            sf::Vertex(world_to_screen({(double)i, -1000}), sf::Color(220, 220, 220)),
            sf::Vertex(world_to_screen({(double)i,  1000}), sf::Color(220, 220, 220))
        };
        
        sf::Vertex hline[] = {
            sf::Vertex(world_to_screen({-1000, (double)i}), sf::Color(220, 220, 220)),
            sf::Vertex(world_to_screen({ 1000, (double)i}), sf::Color(220, 220, 220))
        };

        window.draw(vline, 2, sf::Lines);
        window.draw(hline, 2, sf::Lines);
    }

    // draw walls
    for (const auto& w : engine.get_walls()) {
        sf::Vertex line[] = {
            sf::Vertex(world_to_screen(w.a), sf::Color::Blue),
            sf::Vertex(world_to_screen(w.b), sf::Color::BLue)
        };

        window.draw(line, 2, sf::Lines);
    }

    window.display();
}