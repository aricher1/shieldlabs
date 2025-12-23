#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include "ui/GridRenderer.hpp"
#include <iostream>
#include <fstream>



int main() {

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "XRCT Radiation Shielding Optimization", sf::Style::Titlebar | sf::Style::Default);
    GeometryEngine engine(100, 10.0);
    GridRenderer renderer(window, engine);

    while (window.isOpen()) {
        renderer.handle_events();
        renderer.render();
    }
    
    // print json once exited
    // std::cout << engine.to_json() << std::endl;

}