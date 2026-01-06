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

    if (!ImGui::SFML::Init(window)) {
        return 1;
    }
    
    sf::Clock deltaClock;
    GeometryEngine engine(200, 5.0);
    GridRenderer renderer(window, engine);
    renderer.load_background_image("../assets/floorplans/floorplan.png");

    while (window.isOpen()) {
        renderer.handle_events();
        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(sf::Color::White);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40));

        ImGui::Begin(
            "TopToolbar",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings
        );

        // Example buttons (temporary)
        if (ImGui::Button("Select")) {
            // later: set mode
        }
        ImGui::SameLine();

        if (ImGui::Button("Draw Wall")) {
            // later: set tool
        }   
        ImGui::SameLine();

        if (ImGui::Button("Finalize")) {
            // later: call finalize_blueprint()
        }

        ImGui::End();

        renderer.render();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
