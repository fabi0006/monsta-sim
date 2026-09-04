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
#include <QColor>
#include <QTimerEvent>
#include <QVector2D>
#include <QApplication>

#include "../GUI/MonstaWidget.h"
#include "MonstaMain.h"
#include "MonstaDefs.h"


MonstaMain::MonstaMain() : QThread()
{
    // without this no new thread is created
    moveToThread(this);
    setTerminationEnabled(true);

    // no buttons pushed
    m_GamePad = 255;

    m_X = m_MonstaWidget->m_X;
    m_Y = m_MonstaWidget->m_Y;
}

MonstaMain::MonstaMain(const MonstaMain& mm) : QThread()
{
}

MonstaMain::~MonstaMain()
{
}

MonstaMain& MonstaMain::getInstance()
{
    static MonstaMain instance;
    return instance;
};

void MonstaMain::setMonstaWidget(MonstaWidget* monWid)
{
    m_MonstaWidget = monWid;
}

MonstaWidget* MonstaMain::getMonstaWidget()
{
    return m_MonstaWidget;
}

void MonstaMain::simStartMonsta()
{
    // creates a new threads and executes run()
    start();
}

// starting point of the execution of a new thread
void MonstaMain::run()
{
    //std::cout << "Thread in run: " << thread() << std::endl;
    QApplication::beep();
    std::cout << " ----------- beep ---------" << std::endl;
    startMonsta();
}

void MonstaMain::timerEvent(QTimerEvent * event)
{
    //usleep(100000);
}

void MonstaMain::simUsleep(double usec)
{
    usleep(usec);
}

void MonstaMain::simSetButtonPressed(int button)
{
    // delete bit on position button
    m_GamePad &= ~(1 << button);
}

void MonstaMain::simSetButtonReleased(int button)
{
    // set bit on position button
    m_GamePad |= (1 << button);
}

void MonstaMain::simReset()
{
    m_MonstaWidget->setResetState();
    std::cerr << "Reset is currently not supported" << std::endl;
}

void MonstaMain::simSetPixel(unsigned char x, unsigned char y, unsigned char color)
{
    // set video buffer of monstaWidget on position (x,y) to the converted color
    m_MonstaWidget->m_VideoBuffer[y*MonstaWidget::m_X + x] = m_MonstaWidget->m_ccToRgb->value(color);
    m_MonstaWidget->m_ModifiedBlocks.append(QVector2D(x,y));
}

unsigned char MonstaMain::simGetPixel(unsigned char x, unsigned char y)
{
    // get MONSTA color code (i.e. the key) in video buffer position (x,y)
    QColor color = m_MonstaWidget->m_VideoBuffer[y*MonstaWidget::m_X + x];
    return (unsigned char) m_MonstaWidget->m_rgbToCc->key(color);
}

void MonstaMain::simSetHLine(unsigned char x1, unsigned char x2, unsigned char y, unsigned char color)
{
    for(unsigned char i = x1; i <= x2; i++)
    {
        simSetPixel(i,y,color);
    }
}

void MonstaMain::simSetVLine(unsigned char x, unsigned char y1, unsigned char y2, unsigned char color)
{
    for(unsigned char i = y1; i <= y2; i++)
    {
        simSetPixel(x,i,color);
    }
}

void MonstaMain::simSetScreen(unsigned char color)
{
    QColor qcolor = m_MonstaWidget->m_ccToRgb->value(color);
    m_MonstaWidget->m_VideoBuffer.fill(qcolor);
    m_MonstaWidget->m_repaintAllBlocks = true;
}

unsigned short int MonstaMain::simGetTimestamp()
{
    return m_MonstaWidget->m_TimeCounter;
}

unsigned char MonstaMain::simIsPushed(unsigned char button)
{
    unsigned char any_button = 8;

    if( button == any_button )
    {
        return (m_GamePad < 255);
    }

    unsigned char retVal = !(m_GamePad & (1<<button));

    // reset state for the pushed button (i.e. debounce)
    m_GamePad |= (1<<button);

    return retVal;
}



