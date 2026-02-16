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
        float logoWidth = texSize.x * scale;
        splashSprite.setOrigin({texSize.x / 2.f, texSize.y / 2.f});
        splashSprite.setPosition({250.f, 120.f});

        // load font
        sf::Font splashFont;
        if (!splashFont.openFromFile("../assets/fonts/Inter-Regular.ttf")) { return 1; }

        // status text
        sf::Text statusText(splashFont);
        statusText.setString("Initializing ShieldLabs...");
        statusText.setCharacterSize(14);
        statusText.setFillColor(sf::Color(180, 180, 200));
        
        // center text
        sf::FloatRect textBounds = statusText.getLocalBounds();
        statusText.setOrigin({textBounds.size.x / 2.f, 0.f});
        statusText.setPosition({250.f, 210.f});

        // progress bar dimensions
        float barWidth = logoWidth * 0.75f;
        float barHeight = 16.f;
        float barX = 250.f - barWidth / 2.f;
        float barY = 240.f;

        // progress bar backround
        sf::RectangleShape barBackground;
        barBackground.setSize({barWidth, barHeight});
        barBackground.setFillColor(sf::Color(40, 40, 60));
        barBackground.setPosition({barX, barY});
        
        // fill the progress bar
        sf::RectangleShape barFill;
        barFill.setSize({0.f, barHeight});
        barFill.setFillColor(Cosmetics::PROGRESS_BAR);
        barFill.setPosition({barX, barY});

        sf::Clock timer;

        // splash animation loop
        while (splash.isOpen()) {

            while (auto event = splash.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    splash.close();
                }
            }

            float elapsed = timer.getElapsedTime().asSeconds();
            float duration = 2.2f;
            float t = std::min(elapsed / duration, 1.0f);
            float eased = 1.f - std::pow(1.f - t, 3.f);
            float width = barWidth * eased;
            
            // update progress bar width
            barFill.setSize({width, barHeight});
            splash.clear(sf::Color(11, 11, 20));
            splash.draw(splashSprite);
            splash.draw(statusText);
            splash.draw(barBackground);
            splash.draw(barFill);
            splash.display();

            // close splash when animation is done
            if (t >= 1.0f) {
                splash.close();
            }
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

    if (!ImGui::SFML::Init(window)) {
        return 1;
    }
    
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