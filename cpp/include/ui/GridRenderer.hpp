// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <memory>
#include <cstddef>

#include "geometry/GeometryEngine.hpp"
#include "geometry/WorldBounds.hpp"
#include "calc/CompilerOutput.hpp"
#include "ui/UndoStack.hpp"
#include "app/AppState.hpp"



namespace ui {

    enum class InteractionMode {
        Draw,
        Select,
    };


    enum class Tool {
        None,               // selection mode
        DrawWall,           // wall segment
        PlaceSource,        // source point
        PlaceDose,          // dose point
    };


    struct Selection {
        enum class Type {
            None,
            Wall,
            WallLayer,
            Entity
        };

        Type type = Type::None;
        std::size_t wall_index = 0;
        std::size_t layer_index = 0;
        std::size_t entity_index = 0;

        void clear() {
            type = Type::None;
            wall_index = 0;
            layer_index = 0;
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
            // External systems
            sf::RenderWindow& window;
            geom::GeometryEngine& engine;
            app::AppState& app_state;
            UndoStack undo_stack;

            // Interaction state
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
            std::optional<calc::CalcScene> optimized_scene_cache;
            std::optional<calc::CompilerOutput> optimized_output_cache;

            bool optimization_ran = false;
            bool show_optimization_overlay = false;
            bool show_display_info = false;
            bool show_isotope_popup = false;
            bool show_material_popup = false;
            bool show_help_popup = false;
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

            double pixel_radius_to_world_cm(float px) const;
            Point screen_to_world(sf::Vector2f mouse) const;
            double distance_cm(Point a, Point b) const; // distance between 2 points for a wall segment

        public:
            GridRenderer(sf::RenderWindow& window, geom::GeometryEngine& engine, app::AppState& app_state);

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
            void draw_isotope_tab();
            void handle_events();
            void render_grid_only();
            void render();
    };

} // end namespace ui
