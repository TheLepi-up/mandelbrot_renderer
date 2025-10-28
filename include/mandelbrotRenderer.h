#ifndef MANDELBROTRENDERER_H
#define MANDELBROTRENDERER_H

#include <list>
#include "SFML/Graphics.hpp"
#include "fixedPoint.h"
#include "calculate.h"
#include "config.h"

#define windowWidth 512
#define windowHeight 512

#define maxWidth 2048
#define maxHeight 2048

#define imageWidth 2048
#define imageHeight 2048

typedef struct{
    SetCalc *calc;
    uint64_t depth;
    sf::Image *img;
    short x;
    short y;
} RenderState;


class MandelbrotRenderer
{
private:
    sf::RenderWindow &window;
    ushort *values = nullptr;
    ushort valuesWidth = 0;
    ushort valuesHeight = 0;
    bool changePos = false;
public:
    std::list<RenderState> workItems;
    SetCalc_Impl* calc = nullptr;
    bool stopRendering = false;
    uint64_t windowDepth = 32*256;
    MandelbrotRenderer(sf::RenderWindow &_window, SetCalc_Impl* calc);
    ~MandelbrotRenderer();
    void draw();
    void zoomIn(int posX, int posY);
    void zoomOut(int posX, int posY);
    void beginRenderImage();
    void renderImage(SetCalc_Impl* calc, uint64_t depth, short x, short y);
    void reset();
};




#endif