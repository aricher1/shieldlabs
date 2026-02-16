#pragma once

#include <SFML/Graphics.hpp>



namespace Cosmetics {

    // grid
    inline const sf::Color GRID_COLOR = sf::Color(220, 220, 220);

    // thickness    
    constexpr float WALL_THICKNESS_PX = 4.0f;

    // point sizes
    constexpr float POINT_RADIUS_PX = 4.0f;
    constexpr float POINT_OUTLINE_THICKNESS_PX = 1.5f;

    // wall colours
    inline const sf::Color WALL_NORMAL = sf::Color::Black;
    inline const sf::Color WALL_SELECTED = sf::Color::Green;
    inline const sf::Color WALL_OPTIMIZED = sf::Color(10, 10, 100); // navy

    // points
    constexpr float POINT_RADIUS = 5.0f;
    constexpr float POINT_OUTLINE_THICKNESS = 1.5f;
    inline const sf::Color SOURCE_COLOR = sf::Color::Red;
    inline const sf::Color SOURCE_SELECTED_COLOR = sf::Color::Green;
    inline const sf::Color DOSE_COLOR = sf::Color::Blue;
    inline const sf::Color DOSE_SELECTED_COLOR = sf::Color::Green;
    inline const sf::Color DOSE_LIMIT_PASS = sf::Color(40, 150, 60);
    inline const sf::Color DOSE_LIMIT_FAIL = sf::Color(180, 50, 50);

    // text
    inline const sf::Color LENGTH_TEXT_COLOR = sf::Color::Black;

}