#include <iostream>
#include "mandelbrotRenderer.h"
#include <SFML/System.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

int main(int, char **)
{
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "Mandelbrot Set");
    MandelbrotRenderer renderer(window);
    if(!window.setActive(false))
        cout << "Failed to deactivate graphics in main thread!" << endl;
    boost::thread windowRenderer(&MandelbrotRenderer::draw, &renderer);
    boost::thread imageRenderer;
    
    while (window.isOpen())
    {
        while (const optional<sf::Event> event = window.waitEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                renderer.stopRendering = true;
                windowRenderer.join();
                imageRenderer.interrupt();
                window.close();
            }
            if (const sf::Event::KeyPressed* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch(keyPressed->scancode){
                case sf::Keyboard::Scancode::Space:
                    imageRenderer = boost::thread(&MandelbrotRenderer::renderImage, &renderer);
                    break;
                case sf::Keyboard::Scancode::PageUp:
                    renderer.windowDepth += 256;
                    cout << "WindowDepth: " << renderer.windowDepth << "              " << endl;
                    break;
                    case sf::Keyboard::Scancode::PageDown:
                    if(renderer.windowDepth > 0) renderer.windowDepth -= 256;
                    cout << "WindowDepth: " << renderer.windowDepth <<  "              " << endl;
                    break;
                case sf::Keyboard::Scancode::Escape:
                    renderer.stopRendering = true;
                    windowRenderer.join();
                    renderer.stopRendering = false;
                    renderer.reset();
                    windowRenderer = boost::thread(&MandelbrotRenderer::draw, &renderer);
                    break;
                }
            }
            if (const sf::Event::MouseButtonPressed* mouseButton = event->getIf<sf::Event::MouseButtonPressed>())
            {
                renderer.stopRendering = true;
                windowRenderer.join();
                renderer.stopRendering = false;
                if(!window.setActive(true))
                    cout << "Failed to activate graphics in main thread!" << endl;
                if (mouseButton->button == sf::Mouse::Button::Left)
                    renderer.zoomIn(mouseButton->position.x, mouseButton->position.y);
                else
                    renderer.zoomOut(mouseButton->position.x, mouseButton->position.y);
                if(!window.setActive(false))
                    cout << "Failed to deactivate graphics in main thread!" << endl;
                windowRenderer = boost::thread(&MandelbrotRenderer::draw, &renderer);
            }
        }
    }
}