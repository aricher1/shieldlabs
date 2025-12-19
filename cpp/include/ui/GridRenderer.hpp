#pragma once
#include <SFML/Graphics.hpp>
#include "geometry/GeometryEngine.hpp"



class GridRenderer {

    private:
        sf::RenderWindow& window;
        GeometryEngine& engine;

        sf::Vector2u window_size;

        static constexpr float MIN_PIXELS_PER_CM = 0.1f;
        static constexpr float MAX_PIXELS_PER_CM = 100.0f;

        float pixels_per_cm = 2.0f;
        sf::Vector2f origin_px = {0.0f, 0.0f};

        bool drawing = false;
        Point start_point;

        Point screen_to_world(sf::Vector2i mouse) const;
        sf::Vector2f world_to_screen(Point p) const;


    public:
        GridRenderer(sf::RenderWindow& window, GeometryEngine& engine);

        void handle_events();
        void render();
};