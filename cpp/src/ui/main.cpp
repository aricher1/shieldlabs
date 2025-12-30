#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include "calc/SceneCompiler.hpp"
#include "calc/HitClassification.hpp"
#include "materials/MaterialRegistry.hpp"
#include "ui/GridRenderer.hpp"
#include <iostream>
#include <fstream>

MaterialRegistry material_registry;

int main() {

    if (!material_registry.load_from_file("../assets/materials/materials.yml")) {
        return 1; // fail hard
    }

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "XRCT Radiation Shielding Optimization", sf::Style::Titlebar | sf::Style::Default);
    GeometryEngine engine(100, 10.0);
    GridRenderer renderer(window, engine);

    while (window.isOpen()) {
        renderer.handle_events();
        renderer.render();
    }

}