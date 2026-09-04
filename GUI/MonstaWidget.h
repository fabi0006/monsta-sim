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


#ifndef MONSTAWIDGET_H
#define MONSTAWIDGET_H

#include <QWidget>
#include <QMap>

class QColor;
class QTimer;
class QVector2D;
class QPaintEvent;
class QGraphicsScene;
class MonstaMain;

class MonstaWidget : public QWidget
{
    Q_OBJECT

public:
    MonstaWidget();
    virtual ~MonstaWidget();

    int getState(){
        return m_State;
    }

    void setResetState(){
        m_State = RESET;
    }

    // --- public members ---
    // used to convert a MONSTA color code to a QColor
    QMap<int, QColor>* m_ccToRgb;
    // used to convert a QColor to MONSTA color code
    QMap<int, QColor>* m_rgbToCc;
    // stores the graphics information
    QVector<QColor> m_VideoBuffer;
    // stores the indeces of modified blocks in the video buffer
    QVector<QVector2D> m_ModifiedBlocks;
    bool m_repaintAllBlocks;
    // resolution of simulated Monsta in blocks (one block consists of various pixels)
    static const int m_X = 56;
    static const int m_Y = 42;
    // is incremented every 40 ms (after every TV screen)
    unsigned short int m_TimeCounter;

    // states of the simulator
    enum {
        WAIT_FOR_START, STARTED, WAIT_FOR_STOP, STOPPED, RESET
    };

public slots:
    void startButtonClicked();
    void stopButtonClicked();
    void gridButtonClicked();
    void nextControlStep();
    void startSimulation();

protected:
    bool event(QEvent *e);

private:
    void createColorMap();
    void paintEvent(QPaintEvent* e);

    // timer for simulator control
    QTimer* m_ControlTimer;

    // borders of the drawing area to the edge of the widget
    int m_BorderX;
    int m_BorderY;

    // size of a block in pixels
    int m_PixelsPerBlock;
    bool m_PaintGrid;

    QGraphicsScene* m_Scene;

    // States of the simulator
    int m_State;
};

#endif // MONSTAWIDGET_H
