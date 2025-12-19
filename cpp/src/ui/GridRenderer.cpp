#include "ui/GridRenderer.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>


static constexpr int GRID_CELLS = 100;
static constexpr double CM_PER_CELL = 10.0;



GridRenderer::GridRenderer(sf::RenderWindow& w, GeometryEngine& e) : window(w), engine(e), length_text(font) {
    
    window_size = window.getSize();

    const float world_size = static_cast<float>(GRID_CELLS * CM_PER_CELL);

    grid_view.setSize(sf::Vector2f{world_size, world_size});
    grid_view.setCenter(sf::Vector2f{0.f, 0.f});

    float win_w = static_cast<float>(window_size.x);
    float win_h = static_cast<float>(window_size.y);
    float scale = std::min(win_w, win_h);

    grid_view.setViewport(sf::FloatRect{{(win_w - scale) / win_w / 2.f, (win_h - scale) / win_h / 2.f}, {scale / win_w, scale / win_h}});

    if (!font.openFromFile("assets/fonts/Inter-Regular.otf")) {
        std::cerr << "Failed to load font\n" << std::endl;
    }

    length_text.setFont(font);
    length_text.setCharacterSize(14);
    length_text.setFillColor(sf::Color::Black);

}


Point GridRenderer::screen_to_world(sf::Vector2f mouse) const {
    
    const double world_size_cm = GRID_CELLS * CM_PER_CELL;
    
    const double nx = mouse.x / grid_view.getSize().x;
    const double ny = mouse.y / grid_view.getSize().y;

    return {

        (nx - 0.5) * world_size_cm, (0.5 - ny) * world_size_cm
    
    };
}


Point GridRenderer::snap_to_grid(Point p) const {
    
    const double spacing = cm_per_cell;

    return {

        std::round(p.x_cm / spacing) * spacing,
        std::round(p.y_cm / spacing) * spacing
    
    };
}


double GridRenderer::distance_cm(Point a, Point b) const {
    // euclidean distance
    const double dx = b.x_cm - a.x_cm;
    const double dy = b.y_cm - a.y_cm;

    return std::sqrt(dx * dx + dy * dy);

}


void GridRenderer::handle_events() {
    while (const auto event = window.pollEvent()) {
        
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            
            window_size = {resized->size.x, resized->size.y};

            float win_w = static_cast<float>(window_size.x);
            float win_h = static_cast<float>(window_size.y);
            float scale = std::min(win_w, win_h);
            
            grid_view.setViewport(sf::FloatRect{{(win_w - scale) / win_w / 2.0f, (win_h - scale) / win_h / 2.0f}, {scale / win_w, scale / win_h}});

        }

        if (const auto* move = event->getIf<sf::Event::MouseMoved>()) {
            
            if (!drawing) {
                continue;
            }

            sf::Vector2i mouse_px {move->position.x, move->position.y};

            sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);

            preview_point = snap_to_grid({mouse_world.x, mouse_world.y});

        }


        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            
            if (mouse->button == sf::Mouse::Button::Left) {
                
                sf::Vector2i mouse_px{mouse->position.x, mouse->position.y};

                sf::Vector2f mouse_world = window.mapPixelToCoords(mouse_px, grid_view);

                Point p = snap_to_grid({mouse_world.x, mouse_world.y});

                if (!drawing) {
                    start_point = p;
                    preview_point = p;
                    drawing = true;
                } else {
                    std::cout << "A: " << start_point.x_cm << ", " << start_point.y_cm
                              << "  B: " << p.x_cm << ", " << p.y_cm << "\n";
                    engine.add_wall(start_point, p, 20.0, 1.0);
                    drawing = false;
                }
            }
        }
    }
}


void GridRenderer::render() {
    window.clear(sf::Color::White);
    window.setView(grid_view);

    // draw grid
    const double half = (GRID_CELLS * CM_PER_CELL) / 2.0;

    const double min_x = -half;
    const double max_x = half;
    const double min_y = -half;
    const double max_y = half;

    const double grid_spacing_cm = CM_PER_CELL;

    for (double x = min_x; x <= max_x; x += grid_spacing_cm) {

        sf::Vertex line[2];
        
        line[0].position = sf::Vector2f{static_cast<float>(x), static_cast<float>(min_y)};
        line[1].position = sf::Vector2f{static_cast<float>(x), static_cast<float>(max_y)};

        line[0].color = sf::Color(220, 220, 220);
        line[1].color = sf::Color(220, 220, 220);

        window.draw(line, 2, sf::PrimitiveType::Lines);

    }

    for (double y = min_y; y <= max_y; y += grid_spacing_cm) {

        sf::Vertex line[2];
        
        line[0].position = sf::Vector2f{static_cast<float>(min_x), static_cast<float>(y)};
        line[1].position = sf::Vector2f{static_cast<float>(max_x), static_cast<float>(y)};

        line[0].color = sf::Color(220, 220, 220);
        line[1].color = sf::Color(220, 220, 220);

        window.draw(line, 2, sf::PrimitiveType::Lines);

    }

    if (drawing) {

        sf::Vertex preview[2];

        preview[0].position = sf::Vector2f{static_cast<float>(start_point.x_cm), static_cast<float>(start_point.y_cm)};
        preview[1].position = sf::Vector2f{static_cast<float>(preview_point.x_cm), static_cast<float>(preview_point.y_cm)};

        preview[0].color = sf::Color(0, 0, 0, 120);
        preview[1].color = sf::Color(0, 0, 0, 120);

        window.draw(preview, 2, sf::PrimitiveType::Lines);

        double len_cm = distance_cm(start_point, preview_point);

        sf::Vector2f mid{static_cast<float>((start_point.x_cm + preview_point.x_cm) / 2.0), static_cast<float>((start_point.y_cm + preview_point.y_cm) / 2.0)};

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << len_cm << " cm";

        length_text.setString(ss.str());
        length_text.setPosition(mid);

        window.draw(length_text);

    }

    // draw walls
    for (const auto& w : engine.get_walls()) {
        
        sf::Vertex wall[2];

        wall[0].position = sf::Vector2f{static_cast<float>(w.a.x_cm), static_cast<float>(w.a.y_cm)};
        wall[1].position = sf::Vector2f{static_cast<float>(w.b.x_cm), static_cast<float>(w.b.y_cm)};

        wall[0].color = sf::Color::Black;
        wall[1].color = sf::Color::Black;

        window.draw(wall, 2, sf::PrimitiveType::Lines);
    }

    window.setView(window.getDefaultView());

    window.display();
}