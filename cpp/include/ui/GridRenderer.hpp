#pragma once
#include <SFML/Graphics.hpp>
#include "geometry/GeometryEngine.hpp"



class GridRenderer {

    private:
        sf::RenderWindow& window;
        GeometryEngine& engine;

        float pixels_per_cm = 2.0f;
        sf::Vector2f origin_px = {400.f, 300.f};

        bool drawing = false;
        Point start_point;

        Point screen_to_world(sf::Vector2i mouse) const;
        sf::Vector2f world_to_screen(Point p) const;


    public:
        GridRenderer(sf::RenderWindow& window, GeometryEngine& engine);

        void handle_events();
        void render();
};