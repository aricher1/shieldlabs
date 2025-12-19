#pragma once
#include <SFML/Graphics.hpp>
#include "geometry/GeometryEngine.hpp"



class GridRenderer {

    private:
        sf::RenderWindow& window;
        GeometryEngine& engine;

        sf::View grid_view;
        sf::Vector2u window_size;
        sf::FloatRect grid_viewport;
        sf::Font font;
        sf::Text length_text;    

        int grid_cells = 100;
        double cm_per_cell = 10.0;

        bool drawing = false;
        Point start_point;
        Point preview_point;

        Point screen_to_world(sf::Vector2f mouse) const;
        double distance_cm(Point a, Point b) const; // distance between 2 points for a wall segment

    public:
        GridRenderer(sf::RenderWindow& window, GeometryEngine& engine);

        Point snap_to_grid(Point p) const; 
        void handle_events();
        void render();
};