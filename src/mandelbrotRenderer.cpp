#include "mandelbrotRenderer.h"
#include <iostream>
#include "fixedPoint.h"
#include <time.h>
#include <config.h>
#include <boost/thread/thread.hpp>

MandelbrotRenderer::MandelbrotRenderer(sf::RenderWindow &_window, SetCalc_Impl* calc) : window(_window), calc(calc)
{
}

MandelbrotRenderer::~MandelbrotRenderer()
{
}

sf::Color fromHSB(float angle)
{
    angle *= 6;
    if (angle <= 255)
        return sf::Color(255, (uint8_t)angle, 0);
    else if (angle <= 2 * 256 - 1)
    {
        angle -= 256;
        return sf::Color(255 - (uint8_t)angle, 255, 0);
    }
    else if (angle <= 3 * 256 - 1)
    {
        angle -= 2 * 256;
        return sf::Color(0, 255, (uint8_t)angle);
    }
    else if (angle <= 4 * 256 - 1)
    {
        angle -= 3 * 256;
        return sf::Color(0, 255 - (uint8_t)angle, 255);
    }
    else if (angle <= 5 * 256 - 1)
    {
        angle -= 4 * 256;
        return sf::Color((uint8_t)angle, 0, 255);
    }
    else if (angle <= 6 * 256 - 1)
    {
        angle -= 5 * 256;
        return sf::Color(255, 0, 255 - (uint8_t)angle);
    }
    return sf::Color(0);
}

void MandelbrotRenderer::draw()
{
    time_t t = time(nullptr);
    ushort *newValues = nullptr;
    try{
        while (valuesWidth < maxWidth)
        {
            uint64_t _windowDepth = windowDepth;
            ushort _valuesWidth = valuesWidth;
            ushort _valuesHeight = valuesHeight;
            if (valuesWidth == 0)
            {
                _valuesWidth = 2;
                _valuesHeight = 2;
            }
            else
            {
                _valuesWidth *= 2;
                _valuesHeight *= 2;
            }
            newValues = new ushort [_valuesWidth * _valuesHeight];
            sf::Image image({_valuesWidth, _valuesHeight});
            for (ushort x = 0; x < _valuesWidth; x++)
            {
                std::cout << "Renderer: " << x << " / " << _valuesWidth << "    \r" << std::flush;
                for (ushort y = 0; y < _valuesHeight; y++)
                {
                    if (stopRendering)
                        break;
                    //if (x % 2 == 0 && y % 2 == 0 && _valuesWidth > 2)
                    //{
                    //    newValues[x + y * _valuesWidth] = values[x / 2 + y / 2 * valuesWidth];
                    //    i = newValues[x + y * _valuesWidth];
                    //}
                    //else
                    //{
                    uint64_t i = calc->calculate(
                        (x - _valuesWidth / 2.0) / (_valuesWidth / 2.0),
                        (y - _valuesHeight / 2.0) / (_valuesHeight / 2.0),
                        _windowDepth
                    );
                    newValues[x + y * _valuesWidth] = i + 1;
                    //}
                    if (i == _windowDepth){
                        newValues[x + y * _valuesWidth] = 0;
                        image.setPixel({x, y}, sf::Color::Black);
                    }
                    else
                        image.setPixel({x, y}, fromHSB((i + 1) % 256));
                }
                if (stopRendering)
                    break;
            }
            if (stopRendering)
            {
                delete newValues;
                break;
            }
            else
            {
                delete values;
                values = newValues;
                valuesWidth = _valuesWidth;
                valuesHeight = _valuesHeight;
                sf::Texture t;
                if(t.loadFromImage(image)){
                    sf::Sprite s(t);
                    s.setPosition({0, 0});
                    s.setScale({windowWidth / (float)valuesWidth, windowHeight / (float)valuesHeight});
                    window.draw(s);
                    window.display();
                }
                else{
                    std::cout << "Could not load image" << std::endl;
                }
            }
        }
    }
    catch(boost::thread_interrupted){
        delete newValues;
    }
    window.setActive(false);
    std::cout << "Renderer finished: " << (time(nullptr) - t) << "s" << std::endl;
}

void MandelbrotRenderer::zoomIn(int mousePosX, int mousePosY)
{
    ushort xStart = mousePosX * valuesWidth / windowWidth / 2;
    ushort xEnd = valuesWidth / 2 + xStart;
    ushort yStart = mousePosY * valuesHeight / windowHeight / 2;
    ushort yEnd = valuesHeight / 2 + yStart;
    ushort _valuesWidth = valuesWidth;
    valuesWidth /= 2;
    valuesHeight /= 2;
    sf::Image image({valuesWidth, valuesHeight});
    ushort *newValues = new ushort [valuesWidth * valuesHeight];
    ushort i = 0;
    for (ushort x = xStart; x < xEnd; x++)
    {
        ushort j = 0;
        for (ushort y = yStart; y < yEnd; y++)
        {
            newValues[i + j * valuesWidth] = values[x + y * _valuesWidth];
            if (values[x + y * _valuesWidth] == 0)
                image.setPixel({i, j}, sf::Color::Black);
            else
                image.setPixel({i, j}, fromHSB(values[x + y * _valuesWidth] % 256));
            j++;
        }
        i++;
    }
    delete values;
    values = newValues;
    sf::Texture t;
    if(t.loadFromImage(image)){
        sf::Sprite s(t);
        s.setPosition({0, 0});
        s.setScale({windowWidth / (float)valuesWidth, windowHeight / (float)valuesHeight});
        window.draw(s);
        window.display();
    }
    else{
        std::cout << "Error loading image" << std::endl;
    }
    calc->zoomIn(
        (mousePosX - windowWidth / 2.0) / windowWidth,
        (mousePosY - windowHeight / 2.0) / windowHeight
    );
    std::cout << calc->toString() << std::endl;
}
void MandelbrotRenderer::zoomOut(int mousePosX, int mousePosY)
{
    delete values;
    values = nullptr;
    valuesWidth = 0;
    valuesHeight = 0;
    calc->zoomOut(
        (mousePosX - windowWidth / 2.0) / windowWidth,
        (mousePosY - windowHeight / 2.0) / windowHeight
    );
    std::cout << calc->toString() << std::endl;
}

void MandelbrotRenderer::beginRenderImage(){
    SetCalc_Impl* _calc = new SetCalc_Impl(*calc);
    renderImage(_calc, 2 * windowDepth, (short)(-imageWidth / 2), (short)(-imageHeight / 2));
}
void MandelbrotRenderer::renderImage(SetCalc_Impl* calc, uint64_t depth, short x, short y)
{
    std::list<RenderState>::iterator workItem = workItems.end();
    try{
        sf::Image image({imageWidth, imageHeight});
        if(x != -imageWidth / 2 || y != -imageHeight / 2){
            std::cout << "loading image for state " << calc->toString() << std::endl;
            std::stringstream name;
            name << "img " << calc->toString() << ".png";
            if(!image.loadFromFile(name.str())){
                std::cout << "Failed to load image for state. Starting from the beginning!" << std::endl;
                x = -imageWidth / 2;
                y = -imageHeight / 2;
            }

        }

        workItem =  workItems.emplace(workItems.end(), RenderState{calc, depth, &image, -imageWidth / 2, -imageHeight / 2});

        for (short _x = x; _x < imageWidth / 2; _x++)
        {
            std::cout << "Image: " << _x + imageWidth / 2 << " / " << imageWidth << "        \r" << std::flush;
            for (short _y = y; _y < imageHeight / 2; _y++)
            {
                uint64_t i = calc->calculate(
                    _x * 2.0 / imageWidth,
                    _y * 2.0 / imageHeight,
                    depth
                );
                if (i == depth)
                image.setPixel({(uint)(_x + imageWidth / 2), (uint)(_y + imageHeight / 2)}, sf::Color::Black);
                else
                image.setPixel({(uint)(_x + imageWidth / 2), (uint)(_y + imageHeight / 2)}, fromHSB(i % 256));
                workItem->x = _x;
                workItem->y = _y;
            }
            y = -imageHeight / 2;
        }
        std::stringstream name;
        name << "img " << calc->toString() << ".png";
        if(!image.saveToFile(name.str())){
            std::cout << "Error saving to file" << std::endl;
        }else{
            std::cout << "Saved " << name.str() << std::endl;
            workItems.erase(workItem);
        }
    }
    catch(boost::thread_interrupted){
        delete calc;
        if(workItem != workItems.end())
            workItems.erase(workItem);
    }

}

void MandelbrotRenderer::reset()
{
    calc->reset();
}