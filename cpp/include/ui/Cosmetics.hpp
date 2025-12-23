#pragma once

#include <SFML/Graphics.hpp>


namespace Cosmetics {

    // grid
    inline const sf::Color GRID_COLOR = sf::Color(220, 220, 220);

    // thickness    
    constexpr float WALL_THICKNESS = 6.0f;
    constexpr float OPENING_THICKNESS = 6.0f;

    // wall colours
    inline const sf::Color WALL_NORMAL = sf::Color::Black;
    inline const sf::Color WALL_SELECTED = sf::Color::Green;

    // opening colours
    inline const sf::Color DOOR_COLOR = sf::Color(120, 60, 20);
    inline const sf::Color WINDOW_COLOR = sf::Color(0, 120, 255);
    inline const sf::Color OPEN_COLOR = sf::Color(0, 140, 140);

    // points
    constexpr float POINT_RADIUS = 5.0f;
    constexpr float POINT_OUTLINE_THICKNESS = 1.5f;
    inline const sf::Color SOURCE_COLOR = sf::Color::Red;
    inline const sf::Color SOURCE_SELECTED_COLOR = sf::Color::Green;
    inline const sf::Color DOSE_COLOR = sf::Color::Blue;
    inline const sf::Color DOSE_SELECTED_COLOR = sf::Color::Green;

    // text
    inline const sf::Color LENGTH_TEXT_COLOR = sf::Color::Black;
    inline const sf::Color DOOR_TEXT_COLOR = DOOR_COLOR;
    inline const sf::Color WINDOW_TEXT_COLOR = WINDOW_COLOR;
    inline const sf::Color OPEN_TEXT_COLOR = OPEN_COLOR;

}