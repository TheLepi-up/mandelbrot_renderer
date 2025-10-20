#ifndef MANDELBROTRENDERER_H
#define MANDELBROTRENDERER_H

#include "SFML/Graphics.hpp"
#include "fixedPoint.h"

#define windowWidth 512
#define windowHeight 512

#define maxWidth 2048
#define maxHeight 2048

#define imageWidth 2048
#define imageHeight 2048

class MandelbrotRenderer
{
private:
    sf::RenderWindow &window;
    Fixed_t posX = 0;
    Fixed_t posY = 0.0;
    int scale = -1;
    ushort *values = nullptr;
    ushort valuesWidth = 0;
    ushort valuesHeight = 0;
    bool changePos = false;
public:
    bool stopRendering = false;
    uint64_t windowDepth = 32*256;
    MandelbrotRenderer(sf::RenderWindow &_window);
    ~MandelbrotRenderer();
    void draw();
    void zoomIn(int posX, int posY);
    void zoomOut(int posX, int posY);
    void renderImage();
    void reset();
};




#endif