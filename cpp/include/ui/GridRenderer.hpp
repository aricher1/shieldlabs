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


struct Selection {
    enum class Type {
        None,
        Wall,
        WallLayer,
        Opening,
        Entity
    };

    Type type = Type::None;
    std::size_t wall_index = 0;
    std::size_t layer_index = 0;
    std::size_t opening_index = 0;
    std::size_t entity_index = 0;

    void clear() {
        type = Type::None;
        wall_index = 0;
        layer_index = 0;
        opening_index = 0;
        entity_index = 0;
    }

};


enum class LayerField {
    None,
    Material,
    Thickness
};


struct LayerUIState {
    bool panel_open = false;
    LayerField active_field = LayerField::None;
};


class GridRenderer {

    private:
        sf::RenderWindow& window;
        GeometryEngine& engine;
        UndoStack undo_stack;
        Selection selection;

        InteractionMode interaction_mode = InteractionMode::Draw;
        Tool current_tool = Tool::DrawWall;

        LayerUIState layer_ui;

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

        bool placing_opening = false;
        size_t opening_wall_index = 0;
        double opening_center_t = 0.0;
        double preview_opening_length_cm = 0.0;
        OpeningType opening_type;

        double pixel_radius_to_world_cm(float px) const;
        Point screen_to_world(sf::Vector2f mouse) const;
        double distance_cm(Point a, Point b) const; // distance between 2 points for a wall segment

    public:
        GridRenderer(sf::RenderWindow& window, GeometryEngine& engine);

        void finalize_blueprint(); 
        void handle_select_click(const Point& p);
        void handle_events();
        void render();
};