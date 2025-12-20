#include "geometry/GeometryEngine.hpp"
#include <SFML/Graphics.hpp>
#include "ui/GridRenderer.hpp"
#include <iostream>
#include <fstream>



int main() {

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 800)), "XRCT Grid");
    GeometryEngine engine(100, 10.0);
    GridRenderer renderer(window, engine);

    while (window.isOpen()) {
        renderer.handle_events();
        renderer.render();
    }
    
}