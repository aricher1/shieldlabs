#pragma once
#include <SFML/Graphics.hpp>
#include "geometry/GeometryEngine.hpp"
#include "geometry/WorldBounds.hpp"
#include "calc/CompilerOutput.hpp"
#include "ui/UndoStack.hpp"
#include "app/AppState.hpp"
#include <optional>
#include <string>



enum class InteractionMode {
    Draw,
    Select,
};


enum class Tool {
    None,               // selection mode
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
        AppState& app_state;
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
        sf::Texture background_texture;
        sf::Texture shieldlabs_logo;
        
        std::unique_ptr<sf::Sprite> background_sprite;
        std::string pdf_error_message;
        std::string current_floorplan_png_path;
        std::optional<size_t> inspector_wall_index;
        std::optional<size_t> inspector_source_index;
        std::optional<size_t> inspector_dose_index;
        std::optional<calc::CompilerOutput> last_compiler_output;

        bool scale_calibration_active = false;
        bool scale_has_p1 = false;
        bool scale_has_p2 = false;
        Point scale_p1;
        Point scale_p2;
        double scale_real_distance_cm = 100.0;
        
        void update_viewport();
        float zoom = 1.0f;

        bool drawing = false;
        bool blueprint_finalized = false;
        Point start_point;
        Point preview_point;

        bool placing_opening = false;
        size_t opening_wall_index = 0;
        double opening_center_t = 0.0;
        double preview_opening_length_cm = 0.0;
        ::OpeningType opening_type;

        double pixel_radius_to_world_cm(float px) const;
        Point screen_to_world(sf::Vector2f mouse) const;
        double distance_cm(Point a, Point b) const; // distance between 2 points for a wall segment

    public:
        GridRenderer(sf::RenderWindow& window, GeometryEngine& engine, AppState& app_state);

        bool load_background_image(const std::string& path);

        void finalize_blueprint(); 
        void handle_select_click(const Point& p);
        void draw_project_picker();
        void draw_new_project_setup();
        void draw_toolbar();
        void draw_left_panel();
        void draw_wall_tab();
        void draw_source_tab();
        void draw_dose_tab();
        void handle_events();
        void render_grid_only();
        void render();
};