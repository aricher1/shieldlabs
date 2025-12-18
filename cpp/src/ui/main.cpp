#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include "ui/GridRenderer.hpp"
#include <iostream>
#include <fstream>



int main() {

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "XRCT Grid");
    GeometryEngine engine(1.0);
    GridRenderer renderer(window, engine);

    while (window.isOpen()) {
        renderer.handle_events();
        renderer.render();
    }
    
    /*
    GeometryEngine engine(1.0); // 1 cm grid

    engine.add_wall({0, 0}, {500, 0}, 20.0, 1.0);
    engine.add_wall({500, 0}, {500, 300}, 20.0, 1.0);

    for (const auto& wall : engine.get_walls()) {

        // std::cout << "Wall: (" << wall.a.x_cm << ", " << wall.a.y_cm << ") -> (" << wall.b.x_cm << ", " << wall.b.y_cm << ")\n";
        // std::cout << engine.to_json() << std::endl;

        std::ofstream out("scene.json");
        out << engine.to_json();
        out.close();
    }
    */
}