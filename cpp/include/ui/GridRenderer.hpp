#pragma once
#include <SFML/Graphics.hpp>
#include "geometry/GeometryEngine.hpp"
#include "ui/UndoStack.hpp"
#include <optional>



enum class InteractionMode {
    Draw,
    Select,
};


enum class Tool {
    DrawWall,           // wall segment
    PlaceSource,        // source point
    PlaceDose,          // dose point
    PlaceDoor,          // door
    PlaceWindow,        // window
    PlaceOpen           // open segment in wall (neither door or window)
};


class GridRenderer {

    private:
        sf::RenderWindow& window;
        GeometryEngine& engine;
        UndoStack undo_stack;

        InteractionMode interaction_mode = InteractionMode::Draw;
        Tool current_tool = Tool::DrawWall;

        std::optional<std::size_t> selected_wall_index;
        std::optional<std::size_t> selected_entity_index;

        sf::View grid_view;
        sf::Vector2u window_size;
        sf::FloatRect grid_viewport;
        sf::Font font;
        sf::Text length_text;    

        int grid_cells = 100;
        double cm_per_cell = 1.0;

        bool drawing = false;
        bool blueprint_finalized = false;
        Point start_point;
        Point preview_point;

        Point screen_to_world(sf::Vector2f mouse) const;
        double distance_cm(Point a, Point b) const; // distance between 2 points for a wall segment

    public:
        GridRenderer(sf::RenderWindow& window, GeometryEngine& engine);

        void finalize_blueprint(); 
        void handle_select_click(const Point& p);
        void handle_events();
        void render();
};