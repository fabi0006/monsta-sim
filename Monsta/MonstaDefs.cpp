/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2010-2025, Viktor Seib
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * * list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <iostream>
#include <cmath>
#include <unistd.h>
#include <QTime>
#include "MonstaDefs.h"
#include "MonstaMain.h"

extern "C" {
#include "games/game.h"
}

MonstaMain& monsta = MonstaMain::getInstance();


void reset()
{
    monsta.simReset();
}

// ------------------------------------------------------------------------>> sound.h

void initSound()
{
    //std::cerr << "function: initSound - not yet implemented" << std::endl;
}

void playSong(uint8_t num)
{
    //std::cerr << "function: playSound - not yet implemented" << std::endl;
}

void stopSong()
{
    //std::cerr << "function: stopSound - not yet implemented" << std::endl;
}

// sound.h <<------------------------------------------------------------------------



// ------------------------------------------------------------------------>> graphics.h

uint8_t composeColor(uint8_t odd, uint8_t even)
{
    return (odd & 0xF0) + (even & 0x0F);
}

void setPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if(x >= 0 && x < COLUMNS && y >= 0 && y < LINES)
    {
        monsta.simSetPixel(x, y, color);
    }
}

uint8_t getPixel(uint8_t x, uint8_t y)
{
    return monsta.simGetPixel(x,y);
}

void setHLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color)
{
    monsta.simSetHLine(x1, x2, y, color);
}

void setCompleteHLine(uint8_t y, uint8_t color)
{
    monsta.simSetHLine(0,monsta.m_X-1,y,color);
}

void setVLine(uint8_t x, uint8_t y1, uint8_t y2, uint8_t color)
{
    monsta.simSetVLine(x, y1, y2, color);
}

void setCompleteVLine(uint8_t x, uint8_t color)
{
    monsta.simSetVLine(x,0,monsta.m_Y-1,color);
}

void setScreen(uint8_t color)
{
    monsta.simSetScreen(color);
}

void paintRectangle(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t color)
{
    setHLine(startX,endX,startY,color);
    setHLine(startX,endX,endY,color);
    setVLine(startX,startY,endY,color);
    setVLine(endX,startY,endY,color);
}

void paintFilledRectangle(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t color)
{
    uint8_t i;
    for(i = startY; i <= endY; i++)
    {
        setHLine(startX, endX, i, color);
    }
}

void paintBorders(uint8_t color)
{
    paintRectangle(0,0,monsta.m_X-1,monsta.m_Y-1,color);
}

uint16_t getTimestamp()
{
    return monsta.simGetTimestamp();
}

// graphics.h <<------------------------------------------------------------------------



// ---------------------------------------------------------------------->> graphics3D.h

#define C_LEFT      1
#define C_RIGHT     2
#define C_TOP       4
#define C_BOTTOM    8

#define X_MIN   0
#define X_MAX   (COLUMNS-1)
#define Y_MIN   0
#define Y_MAX   (LINES-1)


// bresenham algorithm from wikipedia.de
void drawLine(uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by, uint8_t color)
{
    int8_t pdx, pdy, ddx, ddy, esf, ess;

    // range in both directions
    int8_t dx = bx - ax;
    int8_t dy = by - ay;

    // find out sign of increment
    int8_t incx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int8_t incy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    if(dx<0) dx = -dx;
    if(dy<0) dy = -dy;

    // find out main (i.e. longer) direction
    if (dx>dy)
    {
        // x is main (fast) direction
        pdx=incx; pdy=0;    // pd - parallel step
        ddx=incx; ddy=incy; // dd - diagonal step
        esf=dy;   ess =dx;  // error steps, fast and slow
    } else
    {
        // y is main (fast) direction
        pdx=0;    pdy=incy; // pd - parallel step
        ddx=incx; ddy=incy; // dd - diagonal step
        esf=dx;   ess =dy;  // error steps, fast and slow
    }

    // inits before loops starts
    uint8_t x = ax;
    uint8_t y = ay;
    int8_t d = ess/2;
    setPixel(x,y,color);

    // compute pixels
    uint8_t t;
    for(t=0; t<ess; ++t) // t counts pixels, ess is also the total pixelsnumber
    {
        // update error term
        d -= esf;
        if(d<0)
        {
            // make error term positive again
            d += ess;
            // step in slow direction, diagonal step
            x += ddx;
            y += ddy;
        } else
        {
            // step in main (fast) direction, parallel step
            x += pdx;
            y += pdy;
        }
        setPixel(x,y,color);
    }
}


uint8_t getOutcode(int8_t x, int8_t y)
{
    uint8_t c = 0;
    if(y > Y_MAX)   c |= 8; // 1000
    if(y < Y_MIN)   c |= 4; // 0100
    if(x > X_MAX)   c |= 2; // 0010
    if(x < X_MIN)   c |= 1; // 0001

    return c;
}

// cohen-sutherland clipping
void clipAndDraw(int8_t ax, int8_t ay, int8_t bx, int8_t by, uint8_t color)
{
    uint8_t c1 = getOutcode(ax, ay);
    uint8_t c2 = getOutcode(bx, by);
    uint8_t c;

    float a, b, d, x, y; // TODO float ist böööse
    a = ay - by;
    b = bx - ax;
    d = ax*by - ay*bx;

    while((c1 | c2) != 0)
    {
        if ((c1 & c2) != 0) return;

        if (c1 == 0) c = c2;
        else c = c1;

        if ((c & C_LEFT) != 0)
        {
            x = X_MIN; y = -(a*x+d)/b;
        }
        else if ((c & C_RIGHT) != 0)
        {
            x = X_MAX; y = -(a*x+d)/b;
        }
        else if ((c & C_TOP) != 0)
        {
            y = Y_MAX; x = -(b*y+d)/a;
        }
        else if ((c & C_BOTTOM) != 0)
        {
            y = Y_MIN; x = -(b*y+d)/a;
        }

        if(c == c1)
        {
            ax = x; ay = y;
            c1 = getOutcode(ax, ay);
        }
        else
        {
            bx = x; by = y;
            c2 = getOutcode(bx, by);
        }

    }

    // since for the MONSTA screen uint8_t is sufficient, cast down variables
    // TODO: letzte Stelle von int16_t wird als Kommastelle interpretiert und hier weggeschnitten
    //ax /= 10; ay /= 10; bx /= 10; by /= 10;
    drawLine(ax, ay, bx, by, color);
}

void drawPolygon(Polygon* pol, uint8_t color)
{
    uint8_t i;
    for(i = 1; i < (*pol).size; i++)
    {
        clipAndDraw((*pol).points[i-1].x,(*pol).points[i-1].y,(*pol).points[i].x,(*pol).points[i].y, color);
    }
    clipAndDraw((*pol).points[0].x,(*pol).points[0].y,(*pol).points[(*pol).size-1].x,(*pol).points[(*pol).size-1].y, color);
}

void fillPolygon(Polygon* pol, uint8_t color)
{
    uint8_t tmp_y;
    uint8_t ymin = 255;
    uint8_t ymax = 0;
    uint8_t i;
    for(i = 0; i < (*pol).size; i++)
    {
        tmp_y = (*pol).points[i].y;
        if(tmp_y > ymax) ymax = tmp_y;
        if(tmp_y < ymin) ymin = tmp_y;
    }

    float t;
    uint8_t xmin = 255;
    uint8_t xmax = 0;
    for(tmp_y = ymin; tmp_y <= ymax; tmp_y++)
    {
        xmin = 255;
        xmax = 0;
        for(i = 1; i < (*pol).size; i++)
        {
            t = (tmp_y - (*pol).points[i-1].y) / (float) ((*pol).points[i].y - (*pol).points[i-1].y);
            if(t >= 0 && t <= 1)
            {
                uint8_t x = (*pol).points[i-1].x + t*((*pol).points[i].x - (*pol).points[i-1].x);
                if(x < xmin) xmin = x;
                if(x > xmax) xmax = x;
            }
        }
        t = (tmp_y - (*pol).points[0].y) / (float) ((*pol).points[(*pol).size-1].y - (*pol).points[0].y);
        if(t >= 0 && t <= 1)
        {
            uint8_t x = (*pol).points[0].x + t*((*pol).points[(*pol).size-1].x - (*pol).points[0].x);
            if(x < xmin) xmin = x;
            if(x > xmax) xmax = x;
        }

        if(xmin <= xmax) setHLine(xmin,xmax,tmp_y,color);
        //if(xmin <= xmax) clipAndDraw(xmin,tmp_y,xmax,tmp_y,color);
    }
}

Point3D crossProduct(Point3D a, Point3D b)
{
    Point3D result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = (a.x* b.z - a.z * b.x) * -1;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

float length(Point3D a)
{
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z); // TODO sqrt in avr
}


float dotProduct(Point3D a, Point3D b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float dotProduct4(Point4D a, Point4D b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

void multMatrix(Matrix4D* m1, Matrix4D* m2, Matrix4D* result)
{
    Point4D tempPoint;

    tempPoint.x = m2->line1.x; tempPoint.y = m2->line2.x; tempPoint.z = m2->line3.x; tempPoint.w = m2->line4.x;
    (*result).line1.x = dotProduct4(m1->line1,tempPoint);
    (*result).line2.x = dotProduct4(m1->line2,tempPoint);
    (*result).line3.x = dotProduct4(m1->line3,tempPoint);
    (*result).line4.x = dotProduct4(m1->line4,tempPoint);

    tempPoint.x = m2->line1.y; tempPoint.y = m2->line2.y; tempPoint.z = m2->line3.y; tempPoint.w = m2->line4.y;
    (*result).line1.y = dotProduct4(m1->line1,tempPoint);
    (*result).line2.y = dotProduct4(m1->line2,tempPoint);
    (*result).line3.y = dotProduct4(m1->line3,tempPoint);
    (*result).line4.y = dotProduct4(m1->line4,tempPoint);

    tempPoint.x = m2->line1.z; tempPoint.y = m2->line2.z; tempPoint.z = m2->line3.z; tempPoint.w = m2->line4.z;
    (*result).line1.z = dotProduct4(m1->line1,tempPoint);
    (*result).line2.z = dotProduct4(m1->line2,tempPoint);
    (*result).line3.z = dotProduct4(m1->line3,tempPoint);
    (*result).line4.z = dotProduct4(m1->line4,tempPoint);

    tempPoint.x = m2->line1.w; tempPoint.y = m2->line2.w; tempPoint.z = m2->line3.w; tempPoint.w = m2->line4.w;
    (*result).line1.w = dotProduct4(m1->line1,tempPoint);
    (*result).line2.w = dotProduct4(m1->line2,tempPoint);
    (*result).line3.w = dotProduct4(m1->line3,tempPoint);
    (*result).line4.w = dotProduct4(m1->line4,tempPoint);
}


void multMatrixVector(Matrix4D* m1, Point3D* m2, Point3D* result)
{
    Point4D tempPoint;

    tempPoint.x = m2->x; tempPoint.y = m2->y; tempPoint.z = m2->z; tempPoint.w = 0; // tempPoint.w = m2->w;
    (*result).x = dotProduct4(m1->line1,tempPoint);
    (*result).y = dotProduct4(m1->line2,tempPoint);
    (*result).z = dotProduct4(m1->line3,tempPoint);
    //(*result).w = dotProduct4(m1->line4,tempPoint);
    (*result).color = (*m2).color;
}

void show3dDemo()
{
    setScreen(BLACK);

    // transformationen
    // (vt): weltkoord. nach kamerakoord.
    // p' = vt * beliebige affine transform. * p    --> modelview transformation
    // p'' = Mortho * (evtl. Mpers. * ) Mrl * p'
    // persp. division, danach viewport transformation


    // camera's eye point
    float ax = 0;
    float ay = 0;
    float az = 0;

    while(1)
    {

        // set camera coordinates
        Point3D a = {ax, ay, az}; // eye point
        Point3D c = {0.0, 0.0, -1.0}; // center point
        Point3D u = {0.0, 1.0, 0.0}; // up vectorPoint3D

        // compute axes of camera in world coordinates
        Point3D tmpPoint; float tmpLength;
        tmpPoint.x = a.x-c.x; tmpPoint.y = a.y-c.y; tmpPoint.z = a.z-c.z;
        tmpLength = length(tmpPoint);
        Point3D zAxis = {tmpPoint.x/tmpLength, tmpPoint.y/tmpLength, tmpPoint.z/tmpLength};

        tmpPoint = crossProduct(u,zAxis);
        tmpLength = length(tmpPoint);
        Point3D xAxis = {tmpPoint.x/tmpLength, tmpPoint.y/tmpLength, tmpPoint.z/tmpLength};

        Point3D yAxis = crossProduct(zAxis,xAxis);

        // create modelview matrix
        Matrix4D modelview = {
            {xAxis.x, xAxis.y, xAxis.z, 0.0},
            {yAxis.x, yAxis.y, yAxis.z, 0.0},
            {zAxis.x, zAxis.y, zAxis.z, 0.0},
            {0.0, 0.0, 0.0, 1.0} };


        // set values for orthographic projection
        float near, far, left, right, top, bottom;
        near = 0.0; far = 100.0; left = -10.0; right = 10.0; top = 10.0; bottom = -10.0;
        // create orthographic projection matrix
        Matrix4D ortho = {
            {2.0f/(right-left), 0.0f, 0.0f, -1.0f*((left+right)/(right-left))},
            {0.0f, 2.0f/(top-bottom), 0.0f, -1.0f*((bottom+top)/(top-bottom))},
            {0.0f, 0.0f, -2.0f/(far-near), -1.0f*((far+near)/(far-near))},
            {0.0f, 0.0f, 0.0f, 1.0f} };

        //    std::cout << "------ ortho hier definiert --------" << std::endl;
        //    std::cout << ortho.line1.x << "  " << ortho.line1.y << "  " << ortho.line1.z << "  " << ortho.line1.w <<  std::endl;
        //    std::cout << ortho.line2.x << "  " << ortho.line2.y << "  " << ortho.line2.z << "  " << ortho.line2.w <<  std::endl;
        //    std::cout << ortho.line3.x << "  " << ortho.line3.y << "  " << ortho.line3.z << "  " << ortho.line3.w <<  std::endl;
        //    std::cout << ortho.line4.x << "  " << ortho.line4.y << "  " << ortho.line4.z << "  " << ortho.line4.w <<  std::endl;


        // test--  pipeline --------------------------
#define ALL_POINTS_LENGTH   5

        // --- input points ----
        Point3D allPoints[ALL_POINTS_LENGTH] = {
            {0, 0, -10, RED}, {-5, 1, -10, SEA_BLUE}, {-4, 5, -10,BLUE},
            {4, 5, -10, ORANGE}, {5, 1, -10, LIGHT_BLUE}
        };

        Polygon polyIn;
        polyIn.size = ALL_POINTS_LENGTH;
        polyIn.color = GREEN;
        polyIn.points = allPoints;

        // --- output points ----
        Point3D outPoints[ALL_POINTS_LENGTH];
        Polygon polyOut;
        polyOut.size = polyIn.size;
        polyOut.color = polyIn.color;



        uint8_t cnt;
        for(cnt = 0; cnt < ALL_POINTS_LENGTH; cnt++)
        {
            Point3D resPoint1;
            multMatrixVector(&modelview,&allPoints[cnt],&resPoint1);
            //std::cout << "modelview: " << (float) (resPoint1.x) << " " << (float) (resPoint1.y) << std::endl;


            Point3D resPoint2;
            multMatrixVector(&ortho,&resPoint1,&resPoint2);
            //std::cout << "ortho: " << (float) (resPoint2.x) << " " << (float) (resPoint2.y) << std::endl;

            // viewport transform
            Point3D screenPoint;
            screenPoint.x = COLUMNS/2 * resPoint2.x + COLUMNS/2;
            screenPoint.y = LINES - (LINES/2 * resPoint2.y + LINES/2);
            screenPoint.color = resPoint2.color;

            //std::cout << "screenPoint: " << (float) (screenPoint.x) << " " << (float) (screenPoint.y) << std::endl << std::endl;
            //setPixel(screenPoint.x, screenPoint.y, screenPoint.color);
            outPoints[cnt] = screenPoint;
        }

        // draw output polygon
        polyOut.points = outPoints;
        for(cnt = 1; cnt < polyOut.size; cnt++)
        {
            clipAndDraw(polyOut.points[cnt-1].x, polyOut.points[cnt-1].y,
                        polyOut.points[cnt].x, polyOut.points[cnt].y, polyOut.color);
        }
        clipAndDraw(polyOut.points[polyOut.size-1].x, polyOut.points[polyOut.size-1].y,
                    polyOut.points[0].x, polyOut.points[0].y, polyOut.color);


        // also draw output points separately
        for(cnt = 0; cnt < polyOut.size; cnt++)
        {
            clipAndDraw(polyOut.points[cnt].x, polyOut.points[cnt].y,
                        polyOut.points[cnt].x, polyOut.points[cnt].y, polyOut.points[cnt].color);
        }


        if(isPushed(ANY_BUTTON))
        {
            setScreen(BLACK);

            if(isPushed(BUTTON_DOWN))
            {
                az += 1;
            }

            if(isPushed(BUTTON_UP))
            {
                az -= 1;
            }

            if(isPushed(BUTTON_LEFT))
            {
                ax += 0.1;
            }

            if(isPushed(BUTTON_RIGHT))
            {
                ax -= 0.1;
            }
        }
        wait(0);// only for the simulator
    }
}

// graphics3D.h <<----------------------------------------------------------------------



// ------------------------------------------------------------------------>> gamePad.h

uint8_t isPushed(uint8_t button)
{
    return monsta.simIsPushed(button);
}

// gamePad.h <<------------------------------------------------------------------------



// ------------------------------------------------------------------------>> symbols.h

void writeBitPattern(uint32_t pattern, uint8_t cols, uint8_t rows, uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t c, r;
    for(r = 0; r < rows; r++)
    {
        for(c = 0; c < cols; c++)
        {
            // if highest bit is set, print pixel
            if(pattern & 0x80000000) setPixel(x+c,y+r,color);
            pattern = pattern << 1;
        }
    }
}

void paintHeart(uint8_t x, uint8_t y, uint8_t color)
{
    writeBitPattern(0x6DFFFFF0,7,4,x,y,color);
    writeBitPattern(0x7C704000,7,3,x,y+4,color);
}

void paintMonstaLogo(uint8_t x, uint8_t y, uint8_t color, uint8_t fillColor)
{
    uint8_t cnt, tmpX;

    // paint eyes and eyes baseline
    tmpX = x+8;
    for(cnt = 0; cnt < 2; cnt++)
    {
        writeBitPattern(0x186642B1,8,4,tmpX,y+0,color);
        writeBitPattern(0xB1B1B181,8,4,tmpX,y+4,color);
        writeBitPattern(0x42241800,8,4,tmpX,y+8,color);
        tmpX += 12;
    }

    // paint body
    // horizontal lines
    setHLine(x+16, x+19, y+9, color);
    setHLine(x+11, x+24, y+10, color);
    setHLine(x+16, x+19, y+10, fillColor);
    setHLine(x+8, x+27, y+11, color);
    setHLine(x+11, x+24, y+11, fillColor);
    setHLine(x+5, x+30, y+12, color);
    setHLine(x+8, x+27, y+12, fillColor);
    setHLine(x+4, x+31, y+13, color);
    setHLine(x+5, x+30, y+13, fillColor);
    // "vertical" lines
    setHLine(x+3, x+32, y+14, color);
    setHLine(x+4, x+31, y+14, fillColor);
    setHLine(x+3, x+32, y+15, color);
    setHLine(x+4, x+31, y+15, fillColor);

    for(cnt = 16; cnt < 26; cnt++)
    {
        setHLine(x+2, x+33, y+cnt, color);
        setHLine(x+3, x+32, y+cnt, fillColor);
    }

    // bottom
    setHLine(x+5, x+30, y+25, color);
    setHLine(x+7, x+28, y+25, fillColor);
    setHLine(x+7, x+28, y+26, color);
    setHLine(x+8, x+27, y+26, fillColor);
    setHLine(x+15, x+20, y+27, color);

    // left and right foot
    tmpX = x;
    for(cnt = 0; cnt < 2; cnt++)
    {
        setHLine(tmpX+1,tmpX+3,y+20,color);
        setVLine(tmpX,y+21,y+25,color);
        setVLine(tmpX+4,y+21,y+25,color);
        setHLine(tmpX+1,tmpX+3,y+26,color);
        setVLine(tmpX+2,y+23,y+25,color);
        setVLine(tmpX+1,y+21,y+25,fillColor);
        setVLine(tmpX+2,y+21,y+22,fillColor);
        setVLine(tmpX+3,y+21,y+25,fillColor);
        tmpX += 31;
    }

    // central feet
    tmpX = x;
    for(cnt = 0; cnt < 2; cnt++)
    {
        setVLine(tmpX+8,y+24,y+28,color);
        setVLine(tmpX+14,y+24,y+28,color);
        setHLine(tmpX+9,tmpX+13,y+23,color);
        setHLine(tmpX+9,tmpX+13,y+29,color);
        setHLine(tmpX+9,tmpX+13,y+28,fillColor);
        setHLine(tmpX+9,tmpX+13,y+27,fillColor);
        setVLine(tmpX+10,y+26,y+28,color);
        setVLine(tmpX+12,y+26,y+28,color);
        tmpX += 13;
    }

    // nose
    setVLine(x+16, y+14, y+15, color);
    setVLine(x+19, y+14, y+15, color);

    // mouth
    setHLine(x+10, x+25, y+17, color);
    setHLine(x+11, x+24, y+17, fillColor);
    setHLine(x+10, x+25, y+18, color);
    setHLine(x+11, x+24, y+18, fillColor);
    setHLine(x+11, x+24, y+19, color);
    setHLine(x+14, x+21, y+19, fillColor);
    setHLine(x+13, x+22, y+20, color);
    setHLine(x+16, x+19, y+20, fillColor);
    setHLine(x+15, x+20, y+21, color);
}

void writeString(char* word, uint8_t x, uint8_t y, uint8_t color)
{
    char c; // holds one character from the string
    uint8_t originalX = x; // holds the original value of x

    // will hold parameters for writeBitPattern function
    uint32_t pattern;
    uint8_t cols, rows;

    // loop over all characters in the word
    uint8_t i = 0;
    for(i = 0; word[i] != '\0'; i++)
    {
        c = word[i];
        if( c >= 'a' ) // "to-upper function"
        {
            c -= 32;
        }

        switch(c)
        {
        case 'A': pattern = LETTER_A; cols = 4; break;
        case 'B': pattern = LETTER_B; cols = 4; break;
        case 'C': pattern = LETTER_C; cols = 3; break;
        case 'D': pattern = LETTER_D; cols = 4; break;
        case 'E': pattern = LETTER_E; cols = 3; break;
        case 'F': pattern = LETTER_F; cols = 3; break;
        case 'G': pattern = LETTER_G; cols = 4; break;
        case 'H': pattern = LETTER_H; cols = 4; break;
        case 'I': pattern = LETTER_I; cols = 3; break;
        case 'J': pattern = LETTER_J; cols = 4; break;
        case 'K': pattern = LETTER_K; cols = 4; break;
        case 'L': pattern = LETTER_L; cols = 3; break;
        case 'M': pattern = LETTER_M; cols = 5; break;
        case 'N': pattern = LETTER_N; cols = 4; break;
        case 'O': pattern = LETTER_O; cols = 4; break;
        case 'P': pattern = LETTER_P; cols = 4; break;
        case 'Q': pattern = LETTER_Q; cols = 4; break;
        case 'R': pattern = LETTER_R; cols = 4; break;
        case 'S': pattern = LETTER_S; cols = 4; break;
        case 'T': pattern = LETTER_T; cols = 3; break;
        case 'U': pattern = LETTER_U; cols = 4; break;
        case 'V': pattern = LETTER_V; cols = 4; break;
        case 'W': pattern = LETTER_W; cols = 5; break;
        case 'X': pattern = LETTER_X; cols = 3; break;
        case 'Y': pattern = LETTER_Y; cols = 4; break;
        case 'Z': pattern = LETTER_Z; cols = 3; break;

        case '0': pattern = DIGIT_0; cols = 4; break;
        case '1': pattern = DIGIT_1; cols = 2; break;
        case '2': pattern = DIGIT_2; cols = 4; break;
        case '3': pattern = DIGIT_3; cols = 4; break;
        case '4': pattern = DIGIT_4; cols = 4; break;
        case '5': pattern = DIGIT_5; cols = 4; break;
        case '6': pattern = DIGIT_6; cols = 4; break;
        case '7': pattern = DIGIT_7; cols = 4; break;
        case '8': pattern = DIGIT_8; cols = 4; break;
        case '9': pattern = DIGIT_9; cols = 4; break;

        case '.': pattern = CHAR_PERIOD; cols = 1; break;
        case ':': pattern = CHAR_COLON; cols = 1; break;
        case '!': pattern = CHAR_EXCLM; cols = 1; break;

        case ' ': x += 2; continue;
        case '_': ;
        case '\n':x = originalX; y = y + 6; continue;
        default: pattern = UNKNOWN_CHAR; cols = 4; break;;
        }

        if( pattern == LETTER_Q ) rows = 6;
        else rows = 5;

        writeBitPattern(pattern, cols, rows, x, y, color);
        // move cursor one letter to the right and add space between two chars
        x = x + cols + 1;
    }
}

// symbols.h <<------------------------------------------------------------------------



// ------------------------------------------------------------------------>> util.h
void convertToString(uint16_t num, char* str, uint8_t length)
{
    switch(length)
    {
    case 6: str[length-6] = (num/10000)+48;
    case 5: str[length-5] = (num%10000)/1000+48;
    case 4: str[length-4] = (num%1000)/100+48;
    case 3: str[length-3] = (num%100)/10+48;
    case 2: str[length-2] = (num%10)+48;
    case 1: str[length-1] = 0;
    }
}

uint16_t absi(int16_t value)
{
    if(value < 0)
    {
        value *= -1;
    }
    return (uint16_t) value;
}

uint8_t getRandomNumber()
{
    static int q = 0;

    if(q == 0)
    {
        //qsrand(QTime::currentTime().msec());
        qsrand(31290);
        q = 1;
    }

    return qrand();
}

void wait(uint16_t msec)
{
    // used to slow down MONSTA thread and omit full CPU usage in the simulator
    if(msec == 0)
    {
        usleep(100000);
        return;
    }
    // used to slow down main loop in MONSTA games (should be lower than thread slow down)
    else if(msec == 1)
    {
        usleep(500);
        return;
    }

    usleep(msec*1000);
}


/**
*	Paints the game pad with the upper left cornet at the given position
*/
static void paintGamePad(uint8_t x, uint8_t y)
{
	setHLine(x,x+12,y,BLACK);
	setHLine(x,x+12,y+6,BLACK);
	setVLine(x,y,y+6,BLACK);
	setVLine(x,y+1,y+5,BLACK);
	setVLine(x+12,y+1,y+5,BLACK);
	
	uint8_t cnt;
	for(cnt = 0; cnt < 5; cnt++)
	{
		setHLine(x+1,x+11,y+1+cnt,WHITE);
	}

	setPixel(x+3,y+2,BLACK);
	setPixel(x+3,y+4,BLACK);
	setHLine(x+2,x+4,y+3,BLACK);

	setPixel(x+6,y+3,YELLOW);
	setPixel(x+8,y+3,RED);
	setPixel(x+10,y+3,RED);
}


/**
*	Displays the main screen of MONSTA
*/
static void showMainScreen()
{
	// background
	setScreen(WHITE);

	// screen borders
    paintBorders(DARK_BLUE);
	// title
    char word[] = "MONSTA";
    writeString(word,14,3, ORANGE);

    // logo
    paintMonstaLogo(10, 10, BLACK, MINT);
}

/**
*	Displays the testscreen for graphics output and game pad test
*/
static void showTestScreen()
{
	// background
	setScreen(WHITE);

	// screen borders
    paintBorders(DARK_BLUE);

	// pixels grid
	uint8_t xPosCounter = 8;
	uint8_t yPosCounter = 2;
	for(xPosCounter = 8; xPosCounter < 47; xPosCounter+=2)
	{
		setPixel(xPosCounter, yPosCounter+0, BLACK);
		setPixel(xPosCounter+1, yPosCounter+1, BLACK);
		setPixel(xPosCounter, yPosCounter+2, BLACK);
		setPixel(xPosCounter+1, yPosCounter+3, BLACK);
		setPixel(xPosCounter, yPosCounter+4, BLACK);
	}

	// stripes borders
	setHLine(7,48,8,BLACK);
	setHLine(7,48,14,BLACK);
	setVLine(7,8,14,BLACK);
	setVLine(48,8,14,BLACK);

	// colored stripes
	xPosCounter = 8;
	yPosCounter = 9;
	uint8_t colorCounter, widthCounter;
	for(colorCounter = BLACK; colorCounter <= WHITE; colorCounter+=17)
	{
		for(widthCounter = 0; widthCounter < 5; widthCounter++)
		{
			setVLine(xPosCounter++, yPosCounter, yPosCounter+4, colorCounter);
		}
	}

	// composed colors (double buffer colors)
	uint8_t xPosStart = 8;
	yPosCounter = 13;
	uint8_t colorCounterInner;
	for(colorCounter = BLACK; colorCounter <= YELLOW; colorCounter+=17)
	{
		xPosCounter = xPosStart + 5;
		yPosCounter += 3;
		for(colorCounterInner = colorCounter+17; colorCounterInner <= WHITE; colorCounterInner+=17)
		{
			for(widthCounter = 0; widthCounter < 5; widthCounter++)
			{
				setVLine(xPosCounter++, yPosCounter, yPosCounter+2, composeColor(colorCounter,colorCounterInner));
			}
		}
		xPosStart += 5;
	}

	// letters
	char word1[] = "2_0_1_0";
	char word2[] = "MS_O T_N A";
	writeString(word1,7,17, RED);
	writeString(word2,12,23, BLUE);

	// gamepad
	uint8_t gamePadX = 24;
	uint8_t gamePadY = 33;
	paintGamePad(gamePadX,gamePadY);

	// gamepad buttons for blinking
	uint16_t buttonPushedTime = 0;
	uint8_t	buttonPushed = 255; // 255 is an invalid value

	// pixel movement and color change
	uint8_t xPos = 25;
	uint8_t yPos = 30;	
	uint8_t pixelColor = MAGENTA;
	uint8_t backColor = getPixel(xPos, yPos);
	setPixel(xPos, yPos, pixelColor);

	while(1)
	{
		// repaint buttons of gamepad
		if(buttonPushed != 255)
		{
			if(getTimestamp() - buttonPushedTime > 1)
			{
				switch(buttonPushed){
				case BUTTON_UP:		setPixel(gamePadX+3,gamePadY+2,BLACK);break;
				case BUTTON_DOWN:	setPixel(gamePadX+3,gamePadY+4,BLACK);break;
				case BUTTON_LEFT:	setPixel(gamePadX+2,gamePadY+3,BLACK);break;
				case BUTTON_RIGHT:	setPixel(gamePadX+4,gamePadY+3,BLACK);break;
				case BUTTON_ZERO:	setPixel(gamePadX+6,gamePadY+3,YELLOW);break;
				case BUTTON_ONE:	setPixel(gamePadX+8,gamePadY+3,RED);break;
				case BUTTON_TWO:	setPixel(gamePadX+10,gamePadY+3,RED);break;
				}
				buttonPushed = 255;
			}
		}

		// handle buttons
		if( isPushed(BUTTON_ANY) )
		{
			setPixel(xPos, yPos, backColor);
			buttonPushedTime = getTimestamp();

			if(isPushed(BUTTON_UP))
			{
				if(yPos >= 1) yPos--;
				setPixel(gamePadX+3,gamePadY+2,GREEN);
				buttonPushed = BUTTON_UP;
			}
			if(isPushed(BUTTON_DOWN))
			{
				if(yPos < LINES-1) yPos++;
				setPixel(gamePadX+3,gamePadY+4,GREEN);
				buttonPushed = BUTTON_DOWN;
			}
			if(isPushed(BUTTON_LEFT))
			{
				if(xPos >= 1) xPos--;
				setPixel(gamePadX+2,gamePadY+3,GREEN);
				buttonPushed = BUTTON_LEFT;
			}
			if(isPushed(BUTTON_RIGHT))
			{
				if(xPos < COLUMNS-1) xPos++;
				setPixel(gamePadX+4,gamePadY+3,GREEN);
				buttonPushed = BUTTON_RIGHT;
			}

			if(isPushed(BUTTON_ZERO))
			{
				pixelColor = backColor;
				setPixel(gamePadX+6,gamePadY+3,GREEN);
				buttonPushed = BUTTON_ZERO;
				// leave loop and go back to main menu
				if(xPos == gamePadX+3 && yPos == gamePadY+3) break;
			}

			if(isPushed(BUTTON_ONE))
			{
				pixelColor+=17;
				if(pixelColor > WHITE) pixelColor = BLACK;
				setPixel(gamePadX+8,gamePadY+3,GREEN);
				buttonPushed = BUTTON_ONE;
			}

			if(isPushed(BUTTON_TWO))
			{
				pixelColor-=17;
				if(pixelColor >= WHITE) pixelColor = WHITE;
				setPixel(gamePadX+10,gamePadY+3,GREEN);
				buttonPushed = BUTTON_TWO;
			}
			backColor = getPixel(xPos, yPos);
			setPixel(xPos, yPos, pixelColor);
		}
        wait(0);// only for the simulator
	}
}

/**
*	Here it all begins
*/
void startMonsta(void)
{
    beginning:

    //    Polygon pol;
    //    pol.size = 4;
    //    Point3D ppp[4] = {{10,20,0}, {30,5,0}, {48,25,0}, {25,35,0}};
    //    pol.points = ppp;
    //
    //    fillPolygon(&pol, BLUE);
    //    drawPolygon(&pol, RED);


    //    // testlinien
    //    clipAndDraw(28,21,28,1);
    //        clipAndDraw(28,21,38,1);
    //        clipAndDraw(28,21,48,1);
    //        clipAndDraw(28,21,48,11);
    //    clipAndDraw(28,21,48,21);
    //        clipAndDraw(28,21,48,31);
    //        clipAndDraw(28,21,48,41);
    //        clipAndDraw(28,21,38,41);
    //    clipAndDraw(28,21,28,41);
    //        clipAndDraw(28,21,18,41);
    //        clipAndDraw(28,21,8,41);
    //        clipAndDraw(28,21,8,31);
    //    clipAndDraw(28,21,8,21);
    //        clipAndDraw(28,21,8,11);
    //        clipAndDraw(28,21,8,1);
    //        clipAndDraw(28,21,18,1);

    initSound();
    showMainScreen();
    wait(500);

    while(1)
    {
        if(isPushed(BUTTON_UP))
        {
            stopSong();
            show3dDemo();
            goto beginning;
        }
        else if(isPushed(BUTTON_DOWN))
        {
            stopSong();
            showTestScreen();
            goto beginning;
        }
        else if(isPushed(BUTTON_ZERO) || isPushed(BUTTON_ONE) || isPushed(BUTTON_TWO))
        {
            stopSong();
            setScreen(BLACK);
            wait(200);
            uint8_t selected = showGameSelectMenu(); // see game.h
            loadGame(selected); // see game.h
            goto beginning;
        }
        wait(0);// only for the simulator
    }
}

// util.h <<------------------------------------------------------------------------
