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


#ifndef MONSTAMAIN_H
#define MONSTAMAIN_H

#include <QThread>

class MonstaWidget;
class QTimer;

// MonstaMain as Singleton

class MonstaMain : public QThread
{
private:
    MonstaMain();
    MonstaMain(const MonstaMain& mm);

public:
    virtual ~MonstaMain();
    static MonstaMain& getInstance();

    void setMonstaWidget(MonstaWidget* monWid);
    MonstaWidget* getMonstaWidget();

    void simStartMonsta();
    void simUsleep(double usec);
    void simSetButtonPressed(int button);
    void simSetButtonReleased(int button);
    void simReset();

    // redifined MONSTA functions for the simulator
    void simSetPixel(unsigned char x, unsigned char y, unsigned char color);
    unsigned char simGetPixel(unsigned char x, unsigned char y);
    void simSetHLine(unsigned char x1, unsigned char x2, unsigned char y, unsigned char color);
    void simSetVLine(unsigned char x, unsigned char y1, unsigned char y2, unsigned char color);
    void simSetScreen(unsigned char color);
    unsigned short int simGetTimestamp();
    unsigned char simIsPushed(unsigned char button);

    // resolution of simulated Monsta in blocks (taken from m_MonstaWidget)
    int m_X;
    int m_Y;

protected:
    void run();
    void timerEvent(QTimerEvent * event);

private:
    MonstaWidget* m_MonstaWidget;
    unsigned char m_GamePad;
};


#endif // MONSTAMAIN_H
