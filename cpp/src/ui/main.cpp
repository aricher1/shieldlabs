#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include "materials/MaterialRegistry.hpp"
#include "ui/GridRenderer.hpp"
#include <iostream>
#include <fstream>

MaterialRegistry material_registry;

int main() {

    if (!material_registry.load_from_file("../assets/materials/materials.yml")) {
        return 1; // fail hard
    }

    // temporary sanity check
    for (int id : {1, 2, 3}) {
        const auto* m = material_registry.get(id);
        if (m) {
            std::cout << "Loaded material: "
                      << m->id << " "
                      << m->key << " "
                      << m->name << "\n";
        }
    }

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "XRCT Radiation Shielding Optimization", sf::Style::Titlebar | sf::Style::Default);
    GeometryEngine engine(100, 10.0);
    GridRenderer renderer(window, engine);

    while (window.isOpen()) {
        renderer.handle_events();
        renderer.render();
    }

}