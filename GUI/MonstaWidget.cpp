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


#include "MonstaWidget.h"
#include "../Monsta/MonstaMain.h"
#include <QPaintEvent>
#include <QPainter>
#include <QVector>
#include <QColor>
#include <QBrush>
#include <QTime>
#include <QTimer>
#include <QPixmap>
#include <iostream>
#include <QMapIterator>
#include <QVector2D>
#include <QGraphicsScene>

MonstaWidget::MonstaWidget()
{
    MonstaMain::getInstance().setMonstaWidget(this);

    m_BorderX = 10;
    m_BorderY = 10;
    m_PixelsPerBlock = 22;
    m_PaintGrid = false;

    createColorMap();
    m_repaintAllBlocks = false;

    m_VideoBuffer.resize(m_X*m_Y);
    m_VideoBuffer.fill(QColor(0,0,0));

    setMinimumSize(m_BorderX*2+m_PixelsPerBlock*m_X+1, m_BorderY*2+m_PixelsPerBlock*m_Y+1);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_ControlTimer = new QTimer();
    connect(m_ControlTimer, SIGNAL(timeout()), this, SLOT(nextControlStep()));
    m_ControlTimer->start(40);

    m_State = STOPPED;
}

MonstaWidget::~MonstaWidget()
{
    delete m_ccToRgb;
    delete m_rgbToCc;
    delete m_ControlTimer;
    delete m_Scene;
}

void MonstaWidget::startButtonClicked()
{
    this->setFocus();

    //std::cout << "start" << std::endl;
    //std::cout << "Thread in gui: " << thread() << std::endl;

    if(m_State == STOPPED)
    {
        m_State = WAIT_FOR_START;
    }
}

void MonstaWidget::stopButtonClicked()
{
    this->setFocus();
    //std::cout << "stop" << std::endl;
    m_State = WAIT_FOR_STOP;

    if(m_State == STARTED)
    {
        m_State = WAIT_FOR_STOP;
    }
}

void MonstaWidget::gridButtonClicked()
{
    this->setFocus();
    m_PaintGrid = !m_PaintGrid;
    if(!m_PaintGrid) m_repaintAllBlocks = true;
    //std::cout << "grid"<< std::endl;
    repaint();
}

void MonstaWidget::nextControlStep()
{
    switch(m_State)
    {
    case WAIT_FOR_START:
        MonstaMain::getInstance().simStartMonsta();
        m_TimeCounter = 0;
        m_State = STARTED;
        break;

    case STARTED:
        m_TimeCounter++;
        repaint();
        break;

    case WAIT_FOR_STOP:
        m_State = STOPPED;
        MonstaMain::getInstance().terminate();
        m_VideoBuffer.fill(QColor(0,0,0));
        repaint();
        break;

    case STOPPED:
        break;

    case RESET:
        m_State = WAIT_FOR_START;
        MonstaMain::getInstance().terminate();
        m_VideoBuffer.fill(QColor(0,0,0));
        repaint();
        //std::cout << "reset start" << std::endl;
        break;
    }
}

void MonstaWidget::paintEvent(QPaintEvent* e){

    e->accept();
    QPainter painter(this);

    if(m_State == STOPPED)
    {
        // draw background of screen area
        painter.setBrush(QBrush(QColor(247,247,247), Qt::SolidPattern));
        painter.drawRect(0, 0, m_X*m_PixelsPerBlock+m_BorderX*2,m_Y*m_PixelsPerBlock+m_BorderY*2);

        painter.setBrush(QBrush(QColor(0,0,0), Qt::SolidPattern));
        painter.drawRect(m_BorderX+1, m_BorderY+1, m_X*m_PixelsPerBlock,m_Y*m_PixelsPerBlock);
    }

    QTime* time = new QTime();
    time->start();

    // create a pixmap the size of a block and repaint blocks
    QPixmap* pix = new QPixmap(m_PixelsPerBlock, m_PixelsPerBlock);
    if(m_repaintAllBlocks)
    {
        for(int y = 0; y < m_Y; y++)
        {
            for(int x = 0; x < m_X; x++)
            {
                // fill pixmap with colors from video buffer
                pix->fill(m_VideoBuffer[x+y*m_X]);
                painter.drawPixmap(x*m_PixelsPerBlock + m_BorderX+1,y*m_PixelsPerBlock + m_BorderY+1, *pix);
            }
        }
        m_repaintAllBlocks = false;
        m_ModifiedBlocks.clear();
    }
    else
    {
        QVector2D block;
        for(int i = 0; i < m_ModifiedBlocks.size(); i++)
        {
            block = m_ModifiedBlocks.at(i);
            pix->fill(m_VideoBuffer[block.x()+block.y()*m_X]);
            painter.drawPixmap(block.x()*m_PixelsPerBlock + m_BorderX+1,block.y()*m_PixelsPerBlock + m_BorderY+1, *pix);
        }
        m_ModifiedBlocks.clear();
    }

    delete pix;
    //if(time->elapsed() > 1) std::cout << "pixels: " << time->elapsed() << std::endl;

    if(m_PaintGrid)
    {
        // draw separating lines for the blocks with pixmaps
        QPixmap* pixX = new QPixmap(m_X*m_PixelsPerBlock+1, 1);
        QPixmap* pixY = new QPixmap(1, m_Y*m_PixelsPerBlock);
        QColor lineColor(255, 0, 0);
        pixX->fill(lineColor);
        pixY->fill(lineColor);

        for(int x = 0; x <= m_X; x++)
        {
            painter.drawPixmap(x*m_PixelsPerBlock + m_BorderX+1, m_BorderY+1, *pixY);
        }
        for(int y = 0; y <= m_Y; y++)
        {
            painter.drawPixmap(m_BorderX+1,y*m_PixelsPerBlock + m_BorderY+1, *pixX);
        }
        delete pixX;
        delete pixY;
    }
    else // delete right and lower border of the grid
    {
        QPixmap* pixX = new QPixmap(m_X*m_PixelsPerBlock+1, 1);
        QPixmap* pixY = new QPixmap(1, m_Y*m_PixelsPerBlock);
        QColor deleteColor(247,247,247);
        pixX->fill(deleteColor);
        pixY->fill(deleteColor);
        painter.drawPixmap(m_X*m_PixelsPerBlock + m_BorderX+1, m_BorderY+1, *pixY);
        painter.drawPixmap(m_BorderX+1,m_Y*m_PixelsPerBlock + m_BorderY+1, *pixX);
        delete pixX;
        delete pixY;
    }

    delete time;
}

void MonstaWidget::createColorMap()
{
    m_ccToRgb = new QMap<int, QColor>();
    m_ccToRgb->insert(  0, QColor(0,0,0)); // BLACK
    m_ccToRgb->insert( 17, QColor(0,0,255)); // BLUE
    m_ccToRgb->insert( 34, QColor(255,0,0)); // RED
    m_ccToRgb->insert( 51, QColor(255,0,255)); // MAGENTA
    m_ccToRgb->insert( 68, QColor(0,255,0)); // GREEN
    m_ccToRgb->insert( 85, QColor(0,255,255)); // CYAN
    m_ccToRgb->insert(102, QColor(255,255,0)); // YELLOW
    m_ccToRgb->insert(119, QColor(255,255,255)); // WHITE

    m_ccToRgb->insert(  1, QColor(0,0,127)); // DARK_BLUE
    m_ccToRgb->insert(  2, QColor(127,0,0)); // DARK_RED
    m_ccToRgb->insert( 18, QColor(127,0,127)); // DARK_MAGENTA = VIOLET
    m_ccToRgb->insert(  4, QColor(0,127,0)); // DARK_GREEN
    m_ccToRgb->insert( 20, QColor(0,127,127)); // DARK_CYAN
    m_ccToRgb->insert( 36, QColor(127,127,0)); // DARK_YELLOW = KHAKI
    m_ccToRgb->insert( 52, QColor(127,127,127)); // DARK_WHITE = GREY

    m_ccToRgb->insert( 53, QColor(127,127,255)); // LIGHT_BLUE
    m_ccToRgb->insert( 54, QColor(255,127,127)); // LIGHT_RED
    m_ccToRgb->insert( 55, QColor(255,127,255)); // LIGHT_MAGENTA
    m_ccToRgb->insert( 86, QColor(127,255,127)); // LIGHT_GREEN = FRESH
    m_ccToRgb->insert( 87, QColor(127,255,255)); // LIGHT_CYAN
    m_ccToRgb->insert(103, QColor(255,255,127)); // LIGHT_YELLOW

    m_ccToRgb->insert( 19, QColor(127,0,255)); // PALE_VIOLET
    m_ccToRgb->insert( 21, QColor(0,127,255)); // SEA_BLUE
    m_ccToRgb->insert( 69, QColor(0,255,127)); // MINT
    m_ccToRgb->insert( 70, QColor(127,255,0)); // GRASS_GREEN
    m_ccToRgb->insert( 35, QColor(255,0,127)); // PINK
    m_ccToRgb->insert( 38, QColor(255,127,0)); // ORANGE


    // ----------------------- duplicate entries to simulate same resulting colors ------------------------------
    // same as color code 18
    m_ccToRgb->insert( 3, QColor(127,0,127)); // DARK_MAGENTA = VIOLET
    // same as color code 20
    m_ccToRgb->insert( 5, QColor(0,127,127)); // DARK_CYAN
    // same as color code 36
    m_ccToRgb->insert( 6, QColor(127,127,0)); // DARK_YELLOW = KHAKI
    // same as color code 52
    m_ccToRgb->insert( 7, QColor(127,127,127)); // DARK_WHITE = GREY

    // same as color code 52
    m_ccToRgb->insert( 22, QColor(127,127,127)); // DARK_WHITE = GREY
    // same as color code 53
    m_ccToRgb->insert( 23, QColor(127,127,255)); // LIGHT_BLUE

    // same as color code 52
    m_ccToRgb->insert( 37, QColor(127,127,127)); // DARK_WHITE = GREY
    // same as color code 54
    m_ccToRgb->insert( 39, QColor(255,127,127)); // LIGHT_RED
    // same as color code 86
    m_ccToRgb->insert( 71, QColor(127,255,127)); // LIGHT_GREEN = FRESH


    // ------- convert QColor to color code ------- no duplicate entries
    m_rgbToCc = new QMap<int, QColor>();
    m_rgbToCc = new QMap<int, QColor>();
    m_rgbToCc->insert(  0, QColor(0,0,0)); // BLACK
    m_rgbToCc->insert( 17, QColor(0,0,255)); // BLUE
    m_rgbToCc->insert( 34, QColor(255,0,0)); // RED
    m_rgbToCc->insert( 51, QColor(255,0,255)); // MAGENTA
    m_rgbToCc->insert( 68, QColor(0,255,0)); // GREEN
    m_rgbToCc->insert( 85, QColor(0,255,255)); // CYAN
    m_rgbToCc->insert(102, QColor(255,255,0)); // YELLOW
    m_rgbToCc->insert(119, QColor(255,255,255)); // WHITE

    m_rgbToCc->insert(  1, QColor(0,0,127)); // DARK_BLUE
    m_rgbToCc->insert(  2, QColor(127,0,0)); // DARK_RED
    m_rgbToCc->insert( 18, QColor(127,0,127)); // DARK_MAGENTA = VIOLET
    m_rgbToCc->insert(  4, QColor(0,127,0)); // DARK_GREEN
    m_rgbToCc->insert( 20, QColor(0,127,127)); // DARK_CYAN
    m_rgbToCc->insert( 36, QColor(127,127,0)); // DARK_YELLOW = KHAKI
    m_rgbToCc->insert( 52, QColor(127,127,127)); // DARK_WHITE = GREY

    m_rgbToCc->insert( 53, QColor(127,127,255)); // LIGHT_BLUE
    m_rgbToCc->insert( 54, QColor(255,127,127)); // LIGHT_RED
    m_rgbToCc->insert( 55, QColor(255,127,255)); // LIGHT_MAGENTA
    m_rgbToCc->insert( 86, QColor(127,255,127)); // LIGHT_GREEN = FRESH
    m_rgbToCc->insert( 87, QColor(127,255,255)); // LIGHT_CYAN
    m_rgbToCc->insert(103, QColor(255,255,127)); // LIGHT_YELLOW

    m_rgbToCc->insert( 19, QColor(127,0,255)); // PALE_VIOLET
    m_rgbToCc->insert( 21, QColor(0,127,255)); // SEA_BLUE
    m_rgbToCc->insert( 69, QColor(0,255,127)); // MINT
    m_rgbToCc->insert( 70, QColor(127,255,0)); // GRASS_GREEN
    m_rgbToCc->insert( 35, QColor(255,0,127)); // PINK
    m_rgbToCc->insert( 38, QColor(255,127,0)); // ORANGE

}

// TODO VS: split event handler
//bool MonstaWidget::keyPressEvent(QKeyEvent *event)
//{

//}

//bool MonstaWidget::keyReleaseEvent(QKeyEvent *event)
//{

//}

bool MonstaWidget::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        // TODO lock variables from monsta thread
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if(keyEvent->key() == Qt::Key_W)
        {
            MonstaMain::getInstance().simSetButtonPressed(0); // BUTTON_UP 0
            return true;
        }
        else if(keyEvent->key() == Qt::Key_A)
        {
            MonstaMain::getInstance().simSetButtonPressed(3); // BUTTON_LEFT 3
            return true;
        }
        else if(keyEvent->key() == Qt::Key_S)
        {
            MonstaMain::getInstance().simSetButtonPressed(2); // BUTTON_DOWN 2
            return true;
        }
        else if(keyEvent->key() == Qt::Key_D)
        {
            MonstaMain::getInstance().simSetButtonPressed(1); // BUTTON_RIGHT 1
            return true;
        }
        else if(keyEvent->key() == Qt::Key_J)
        {
            MonstaMain::getInstance().simSetButtonPressed(4); // BUTTON_ZERO 4
            return true;
        }
        else if(keyEvent->key() == Qt::Key_K)
        {
            MonstaMain::getInstance().simSetButtonPressed(5); // BUTTON_ONE 5
            return true;
        }
        else if(keyEvent->key() == Qt::Key_L)
        {
            MonstaMain::getInstance().simSetButtonPressed(6); // BUTTON_TWO 6
            return true;
        }
    }

    if (event->type() == QEvent::KeyRelease)
    {
        // TODO lock variables from monsta thread
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_W)
        {
            MonstaMain::getInstance().simSetButtonReleased(0); // BUTTON_UP 0
            return true;
        }
        else if(keyEvent->key() == Qt::Key_A)
        {
            MonstaMain::getInstance().simSetButtonReleased(3); // BUTTON_LEFT 3
            return true;
        }
        else if(keyEvent->key() == Qt::Key_S)
        {
            MonstaMain::getInstance().simSetButtonReleased(2); // BUTTON_DOWN 2
            return true;
        }
        else if(keyEvent->key() == Qt::Key_D)
        {
            MonstaMain::getInstance().simSetButtonReleased(1); // BUTTON_RIGHT 1
            return true;
        }
        else if(keyEvent->key() == Qt::Key_J)
        {
            MonstaMain::getInstance().simSetButtonReleased(4); // BUTTON_ZERO 4
            return true;
        }
        else if(keyEvent->key() == Qt::Key_K)
        {
            MonstaMain::getInstance().simSetButtonReleased(5); // BUTTON_ONE 5
            return true;
        }
        else if(keyEvent->key() == Qt::Key_L)
        {
            MonstaMain::getInstance().simSetButtonReleased(6); // BUTTON_TWO 6
            return true;
        }
    }
    return QWidget::event(event);
}


