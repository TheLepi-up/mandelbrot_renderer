#include "mandelbrotRenderer.h"
#include <iostream>
#include "fixedPoint.h"
#include <time.h>

#define SCALE_PREC (scale + 32)
#define _SCALE_PREC (_scale + 32)

inline uint64_t calculate(Fixed_t x, Fixed_t y, uint64_t depth, size_t precision){
    uint64_t i = 0;
    Fixed_t z_re(precision), z_im(precision), old_z_re(precision), z_im_sq(precision);
    Fixed_t c_re(x), c_im(y);
    Fixed_t ptwo(2, 0);
    Fixed_t ntwo(-2, 0);
    while (z_re > ntwo && z_re < ptwo && z_im > ntwo && z_im < ptwo && i < depth)
    {
        //z_re = z_re * z_re - z_im * z_im + c_re;
        //z_im = ((old_z_re * z_im) << 1) + c_im;
        //old_z_re = z_re;
        //i++;
        
        z_re *= z_re;
        z_im_sq.copyFrom(z_im);
        z_im_sq *= z_im;
        z_re -= z_im_sq;
        z_re += c_re;
        
        z_im *= old_z_re;
        z_im <<= 1;
        z_im += c_im;

        old_z_re.copyFrom(z_re);
        i++;
    }
    // long double z0 = 0.0;
    // long double z1 = 0.0;
    // long double z2 = 0.0;
    // long double z3 = 0.0;
    // long double c0 = x;
    // long double c1 = 0.0;
    // long double c2 = y;
    // long double c3 = 0.0;
    // while (abs(z0) < 2 && abs(z1) < 2 && abs(z2) < 2 && i < depth)
    // {
    //     long double t0 = z0*z0 - 2*z1*z3 - z2*z2 + c0;
    //     long double t1 = 2*z0*z1 - 2*z2*z3 + c1;
    //     long double t2 = 2*z0*z2 + z1*z1 - z3*z3 + c2;
    //     long double t3 = 2*z0*z3 + 2*z1*z2 + c3;
    //     z0 = t0;
    //     z1 = t1;
    //     z2 = t2;
    //     z3 = t3;
    //     i++;
    // }
    return i;
}


MandelbrotRenderer::MandelbrotRenderer(sf::RenderWindow &_window) : window(_window)
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
    int _scale = scale;
    uint64_t _windowDepth = windowDepth;
    while (valuesWidth < maxWidth)
    {
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
        ushort *newValues = new ushort [_valuesWidth * _valuesHeight];
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
                Fixed_t  xoffFix((x - _valuesWidth / 2.0) / (_valuesWidth / 2.0), _SCALE_PREC);
                Fixed_t  yoffFix((y - _valuesHeight / 2.0) / (_valuesHeight / 2.0), _SCALE_PREC);
                xoffFix >>= _scale;
                yoffFix >>= _scale;
                    uint64_t i = calculate(
                        posX + xoffFix, posY + yoffFix,
                        _windowDepth,
                        _SCALE_PREC
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
    window.setActive(false);
    std::cout << "Renderer finished: " << (time(nullptr) - t) << "s" << std::endl;
}

void MandelbrotRenderer::zoomIn(int mousePosX, int mousePosY)
{
    ushort xStart = mousePosX / 2 * valuesWidth / windowWidth;
    ushort xEnd = valuesWidth / 2 + xStart;
    ushort yStart = mousePosY / 2 * valuesHeight / windowHeight;
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
    posX += Fixed_t((mousePosX - windowWidth / 2.0) / windowWidth, SCALE_PREC) >> scale;
    posY += Fixed_t((mousePosY - windowHeight / 2.0) / windowHeight, SCALE_PREC) >> scale;
    scale ++;
    std::cout << "x: " << posX.toHex() << " y: " << posY.toHex() << " scale: 2^" << -scale << std::endl;
}
void MandelbrotRenderer::zoomOut(int mousePosX, int mousePosY)
{
    delete values;
    values = nullptr;
    valuesWidth = 0;
    valuesHeight = 0;
    posX += Fixed_t((mousePosX - windowWidth / 2.0) / windowWidth, SCALE_PREC) >> scale;
    posY += Fixed_t((mousePosY - windowHeight / 2.0) / windowHeight, SCALE_PREC) >> scale;
    scale --;
    std::cout << "x: " << posX.toHex() << " y: " << posY.toHex() << " scale: 2^" << -scale << std::endl;
}

void MandelbrotRenderer::renderImage()
{
    Fixed_t _posX = posX;
    Fixed_t _posY = posY;
    int _scale = scale;
    uint64_t imageDepth = 2 * windowDepth;
    sf::Image image({imageWidth, imageHeight});
    for (short x = -imageWidth / 2; x < imageWidth / 2; x++)
    {
        std::cout << "Image: " << x + imageWidth / 2 << " / " << imageWidth << "        \r" << std::flush;
        for (short y = -imageHeight / 2; y < imageHeight / 2; y++)
        {
            uint64_t i = calculate(
                _posX + (Fixed_t(x * 2.0 / imageWidth, _SCALE_PREC) >> _scale),
                _posY + (Fixed_t(y * 2.0 / imageHeight, _SCALE_PREC) >> _scale),
                imageDepth, _SCALE_PREC
            );
            if (i == imageDepth)
                image.setPixel({(uint)(x + imageWidth / 2), (uint)(y + imageHeight / 2)}, sf::Color::Black);
            else
                image.setPixel({(uint)(x + imageWidth / 2), (uint)(y + imageHeight / 2)}, fromHSB(i % 256));
        }
    }
    std::stringstream name;
    name << "img x:" << _posX.toHex().c_str() << "_y:" << _posY.toHex().c_str() << "_scale:" << _scale << ".png";;
    if(!image.saveToFile(name.str())){
        std::cout << "Error saving to file" << std::endl;
    }else{
        std::cout << "Saved " << name.str() << std::endl;
    }
}

void MandelbrotRenderer::reset()
{
    scale = -1;
    posX = Fixed_t(SCALE_PREC);
    posY = Fixed_t(SCALE_PREC);
}