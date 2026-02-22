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
#include "ui/GridRenderer.hpp"
#include "ui/Cosmetics.hpp"
#include "app/AppState.hpp"



material::MaterialRegistry material_registry;
isotope::IsotopeRegistry isotope_registry;


using namespace app;
using namespace geom;
using namespace ui;



int main() {

    // Load materials
    if (!material_registry.load_from_file("../assets/materials/materials.yml")) { return 1; }
    
    // Load isotopes
    if (!isotope_registry.load_from_file("../assets/isotopes/isotopes.yml")) { return 1; }

    // Splash window
    {
        sf::RenderWindow splash(sf::VideoMode({500, 320}), "", sf::Style::None);
        splash.setFramerateLimit(60);
        
        // center
        auto desktop = sf::VideoMode::getDesktopMode();
        sf::Vector2u size = splash.getSize();
        splash.setPosition({static_cast<int>(desktop.size.x / 2 - size.x / 2), static_cast<int>(desktop.size.y / 2 - size.y / 2)});

        // load logo
        sf::Texture splashTexture;
        if (!splashTexture.loadFromFile("../assets/logos/ShieldLabsTitleLogoTransparent.png")) { return 1; }
        
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
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "", sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    
    // Icon
    sf::Image icon;
    if (icon.loadFromFile("../assets/logos/ShieldLabsLogo.png")) {
        window.setIcon(icon);
    }
    
    // Application state
    AppState app_state;

    if (!ImGui::SFML::Init(window)) { return 1; }
    
    sf::Clock deltaClock;
    GeometryEngine engine(800, 5.0);
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