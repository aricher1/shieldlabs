#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include "ui/GridRenderer.hpp"
#include <iostream>
#include <fstream>



int main() {

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "XRCT Radiation Shielding Optimization", sf::Style::Titlebar | sf::Style::Close);
    GeometryEngine engine(100, 10.0);
    GridRenderer renderer(window, engine);

    /* load test JSON
    {
        std::string saved = engine.to_json();

        GeometryEngine test_engine(1, 1);
        bool ok = test_engine.load_from_json(saved);

        assert(ok);
        assert(test_engine.get_walls().size() == engine.get_walls().size());

        engine = test_engine;

    }
    end of test block */

    while (window.isOpen()) {
        renderer.handle_events();
        renderer.render();
    }
    
    std::cout << engine.to_json() << std::endl; // print JSON once exited

}