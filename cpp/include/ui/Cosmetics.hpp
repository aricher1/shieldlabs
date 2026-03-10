// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <SFML/Graphics.hpp>



namespace Cosmetics {

    // app theme
    inline const sf::Color APP_BACKGROUND = sf::Color(11, 11, 20);
    inline const sf::Color GRID_BACKGROUND = sf::Color(240, 240, 243);

    // splash window
    inline const sf::Color PROGRESS_BAR = sf::Color(10, 10, 100);

    // grid
    inline const sf::Color GRID_COLOR = sf::Color(210, 210, 214);

    // thickness    
    constexpr float WALL_THICKNESS_PX = 4.0f;

    // point sizes
    constexpr float POINT_RADIUS_PX = 4.0f;
    constexpr float POINT_OUTLINE_THICKNESS_PX = 1.5f;

    // wall colours
    inline const sf::Color WALL_NORMAL = sf::Color::Black;
    inline const sf::Color WALL_SELECTED = sf::Color::Green;
    inline const sf::Color WALL_OPTIMIZED = sf::Color(10, 10, 100);

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

} // end namespace Comsmetics
