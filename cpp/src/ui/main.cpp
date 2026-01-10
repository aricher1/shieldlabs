#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <nlohmann/json.hpp>
#include "calc/SceneCompiler.hpp"
#include "calc/HitClassification.hpp"
#include "materials/MaterialRegistry.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "ui/GridRenderer.hpp"
#include "app/AppState.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

MaterialRegistry material_registry;
IsotopeRegistry isotope_registry;

int main() {

    // Load materials
    if (!material_registry.load_from_file("../assets/materials/materials.yml")) {
        return 1;
    }

    // Load isotopes
    if (!isotope_registry.load_from_file("../assets/isotopes/isotopes.yml")) {
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "XRCT Radiation Shielding Optimization", sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    AppState app_state;

    if (!ImGui::SFML::Init(window)) {
        return 1;
    }
    
    sf::Clock deltaClock;
    GeometryEngine engine(200, 5.0);
    GridRenderer renderer(window, engine, app_state);
    std::filesystem::create_directories("cache");

    while (window.isOpen()) {
        renderer.handle_events();
        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(sf::Color::White);
        renderer.render();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
