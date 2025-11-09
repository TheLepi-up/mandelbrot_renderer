#include <iostream>
#include "mandelbrotRenderer.h"
#include <SFML/System.hpp>
#include <boost/thread/thread.hpp>
#include <fixed_t_calc.h>
#include <list>
#include <sstream>
#include <fstream>
#include <cstdio>

using namespace std;

MandelbrotRenderer* renderer;
SetCalc_Impl calc;

void saveState(){
    ostringstream altname;
    altname << "state" << time(0) << ".tsv";
    rename("state.tsv", altname.str().c_str());
    ofstream outfile("state.tsv");
    ostream *metadatafile = &outfile;
    
    if(!outfile.is_open()){
        std::cout << "Failed to write metadata to state.csv! Writing to stdout instead." << endl;
        metadatafile = &cout;
    }
    *metadatafile << 
        renderer->calc->toString() << 
        "\t" << 
        renderer->windowDepth << 
        "\t\t\n";
    
    for(list<RenderState>::iterator it = renderer->workItems.begin(); it != renderer->workItems.end(); it++){
        std::stringstream name;
        short x = it->x;
        short y = it->y;
        name << "img " << it->calc->toString() << ".png";
        if(!it->img->saveToFile(name.str())){
            std::cout << "Error saving to file" << std::endl;
        }else{
            std::cout << "Saved " << name.str() << std::endl;
        }
        *metadatafile << it->calc->toString() << "\t" << it->depth << "\t" << x << "\t" << y << "\n";
    }
    outfile.close();
    cout << "state saved" << endl;
}

inline void resumeState(){
    ifstream state("state.tsv");
    if(state.is_open()){
        cout << "resuming from state.tsv" << endl;
        string line;
        while(!getline(state, line).eof()){
            size_t dIdx = line.find('\t') + 1;
            size_t xIdx = line.find('\t', dIdx) + 1;
            size_t yIdx = line.find('\t', xIdx) + 1;
            if(dIdx == 0 || xIdx == 0 || yIdx == 0){
                if(line.size() != 0)
                    cout << "Ignoring line: " << line;
                continue;
            }
            string pos = line.substr(0, dIdx - 1);
            try{
                uint64_t depth = std::stoull(line.substr(dIdx, xIdx - dIdx));
                if(xIdx + 1 == yIdx){
                    //GUI state
                    renderer->calc->fromStr(pos);
                    renderer->windowDepth = depth;
                    continue;
                }
                short x = std::stoi(line.substr(xIdx, yIdx - xIdx));
                short y = std::stoi(line.substr(yIdx));
                boost::thread(&MandelbrotRenderer::renderImage, renderer, new SetCalc_Impl(pos), depth, x, y);
            }
            catch(invalid_argument &e)
            {
                cout << "error parsing saved state:" << 
                    " x: '" << line.substr(xIdx, yIdx - xIdx) << 
                    "' y:'" << line.substr(yIdx) << 
                    "' depth: '" << line.substr(dIdx, xIdx - dIdx) << 
                    "' pos: '" << pos << "'\nError: " << e.what() << endl;
            }
        }

    }
}


int main(int argc, char **argv){
    bool resume = true;
    if(argc == 2){
        resume = (bool)std::strcmp(argv[1], "-no-resume");
        if(!std::strncmp(argv[1], "x:", 2)){
            calc.fromStr(string(argv[1]));
            resume = false;
        }
    }
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "Mandelbrot Set");
    renderer = new MandelbrotRenderer(window, &calc);
    std::atexit(saveState);
    if(resume)
        resumeState();
    
    if(!window.setActive(false))
        cout << "Failed to deactivate graphics in main thread!" << endl;
    boost::thread windowRenderer(&MandelbrotRenderer::draw, renderer);
    boost::thread imageRenderer;
    
    while (window.isOpen())
    {
        while (const optional<sf::Event> event = window.waitEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                renderer->stopRendering = true;
                windowRenderer.join();
                imageRenderer.interrupt();
                window.close();
            }
            if (const sf::Event::KeyPressed* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch(keyPressed->scancode){
                case sf::Keyboard::Scancode::Space:
                    imageRenderer = boost::thread(&MandelbrotRenderer::beginRenderImage, renderer);
                    break;
                case sf::Keyboard::Scancode::PageUp:
                    renderer->windowDepth += 256;
                    cout << "WindowDepth: " << renderer->windowDepth << "              " << endl;
                    break;
                    case sf::Keyboard::Scancode::PageDown:
                    if(renderer->windowDepth > 0) renderer->windowDepth -= 256;
                    cout << "WindowDepth: " << renderer->windowDepth <<  "              " << endl;
                    break;
                case sf::Keyboard::Scancode::Escape:
                    renderer->stopRendering = true;
                    windowRenderer.join();
                    renderer->stopRendering = false;
                    renderer->reset();
                    windowRenderer = boost::thread(&MandelbrotRenderer::draw, renderer);
                    break;
                case sf::Keyboard::Scancode::S:
                    saveState();
                default:
                    break;
                }
            }
            if (const sf::Event::MouseButtonPressed* mouseButton = event->getIf<sf::Event::MouseButtonPressed>())
            {
                renderer->stopRendering = true;
                windowRenderer.join();
                renderer->stopRendering = false;
                if(!window.setActive(true))
                    cout << "Failed to activate graphics in main thread!" << endl;
                if (mouseButton->button == sf::Mouse::Button::Left)
                    renderer->zoomIn(mouseButton->position.x, mouseButton->position.y);
                else
                    renderer->zoomOut(mouseButton->position.x, mouseButton->position.y);
                if(!window.setActive(false))
                    cout << "Failed to deactivate graphics in main thread!" << endl;
                windowRenderer = boost::thread(&MandelbrotRenderer::draw, renderer);
            }
        }
    }
}