// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
//
//     _____ __  ________________   ____  __    ___    ____  _____
//    / ___// / / /  _/ ____/ / /  / __ \/ /   /   |  / __ // ___/
//    \__ \/ /_/ // // __/ / /    / / / / /   / /| | / __  |\__ \
//   ___/ / __  // // /___/ /___ / /_/ / /___/ ___ |/ /_/ /___/ /
//  /____/_/ /_/___/_____/_____//_____/_____/_/  |_/____//____/
//
// ============================================================================================


#include <filesystem>
#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <nlohmann/json.hpp>

#include "geometry/GeometryEngine.hpp"
#include "calc/SceneCompiler.hpp"
#include "calc/HitClassification.hpp"
#include "materials/MaterialRegistry.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "utils/AppPaths.hpp"
#include "ui/GridRenderer.hpp"
#include "ui/Cosmetics.hpp"
#include "app/AppState.hpp"


material::MaterialRegistry material_registry;
isotope::IsotopeRegistry isotope_registry;


using namespace app;
using namespace geom;
using namespace ui;

namespace {

    void apply_ui_theme() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.GrabRounding = 0.0f;
        style.PopupRounding = 0.0f;
        style.ScrollbarRounding = 0.0f;
        style.FramePadding = ImVec2(8.f, 5.f);
        style.ItemSpacing = ImVec2(8.f, 6.f);

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.0f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.0f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.16f, 0.20f, 1.0f);
        colors[ImGuiCol_Border] = ImVec4(0.30f, 0.33f, 0.40f, 0.65f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.25f, 0.30f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.31f, 0.37f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.31f, 0.35f, 0.43f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.23f, 0.27f, 0.33f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 0.34f, 0.42f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.24f, 0.30f, 1.0f);
        colors[ImGuiCol_Header] = ImVec4(0.21f, 0.26f, 0.34f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.34f, 0.44f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.19f, 0.23f, 0.31f, 1.0f);
        colors[ImGuiCol_Tab] = ImVec4(0.17f, 0.20f, 0.25f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.29f, 0.37f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.21f, 0.26f, 0.34f, 1.0f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.17f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.17f, 1.0f);
    }

} // namespace


int main() {
    const std::filesystem::path assets_dir = utils::find_assets_dir();

    // Load materials
    if (!material_registry.load_from_file((assets_dir / "materials/materials.yml").string())) { return 1; }
    
    // Load isotopes
    if (!isotope_registry.load_from_file((assets_dir / "isotopes/isotopes.yml").string())) { return 1; }

    // Splash window
    {
        sf::RenderWindow splash(sf::VideoMode({500, 320}), "ShieldLabs Splash", sf::Style::None);
        splash.setFramerateLimit(60);
        
        // center
        auto desktop = sf::VideoMode::getDesktopMode();
        sf::Vector2u size = splash.getSize();
        splash.setPosition({static_cast<int>(desktop.size.x / 2 - size.x / 2), static_cast<int>(desktop.size.y / 2 - size.y / 2)});

        // load logo
        sf::Texture splashTexture;
        if (!splashTexture.loadFromFile((assets_dir / "logos/ShieldLabsTitleLogoTransparent.png").string())) { return 1; }
        
        // scale logo
        sf::Sprite splashSprite(splashTexture);
        sf::Vector2u texSize = splashTexture.getSize();
        float scaleX = 500.f / texSize.x;
        float scaleY = 340.f / texSize.y;
        float scale = std::min(scaleX, scaleY);
        splashSprite.setScale({scale, scale});

        // center
        splashSprite.setOrigin({texSize.x / 2.f, texSize.y / 2.f});
        splashSprite.setPosition({250.f, 160.f});

        sf::Clock timer;

        // splash animation loop
        while (splash.isOpen()) {

            while (auto event = splash.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    splash.close();
                }
            }

            if (timer.getElapsedTime().asSeconds() >= 1.5f) { splash.close(); }

            splash.clear(sf::Color::Black);
            splash.draw(splashSprite);
            splash.display();
        }
    }
    
    // Main window
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "ShieldLabs", sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    
    // Icon
    sf::Image icon;
    if (icon.loadFromFile((assets_dir / "logos/ShieldLabsLogo.png").string())) {
        window.setIcon(icon);
    }
    
    // Application state
    AppState app_state;

    if (!ImGui::SFML::Init(window)) { return 1; }
    apply_ui_theme();
    
    sf::Clock deltaClock;
    GeometryEngine engine(800, 5.0);
    GridRenderer renderer(window, engine, app_state);
    std::filesystem::create_directories("cache");

    while (window.isOpen()) {
        renderer.handle_events();
        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(Cosmetics::APP_BACKGROUND);
        renderer.render();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
