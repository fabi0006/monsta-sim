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
 
 
#include "game.h"
#include "glider.h"

#define GLIDER_GROUND_COLOR     LIGHT_GREEN
#define GLIDER_GROUND_LINE      GRASS_GREEN
#define GLIDER_COLOR_CENTER     VIOLET
#define GLIDER_RING_COLOR       RED
#define GLIDER_BACKGROUND       LIGHT_YELLOW //CYAN

#define GLIDER_3D_BACKGROUND    WHITE
#define GLIDER_3D_GROUND_COLOR  WHITE // LIGHT_YELLOW
#define GLIDER_3D_COLOR_LEFT    CYAN
#define GLIDER_3D_COLOR_RIGHT   LIGHT_RED
#define GLIDER_3D_COLOR_CENTER  GRAY

#define UPDATE_LIMIT    2
static uint8_t update = 0; // used to slow down calculation of the next step: outer loop
static uint8_t update2 = 0;// ... : inner loop
static uint8_t gameSpeed = 2; // used to slow down or increase game speed (high values = slow)


struct gameStateStruct
{
    uint8_t paused: 1;
    uint8_t gameOver: 1;
    uint8_t nextLevel: 1;
    uint8_t show3d: 3;
    uint8_t paintLeftColor: 1;
    uint8_t paintCenterColor: 1;
    uint8_t non3DRingColor;
};
typedef struct gameStateStruct gameStateStruct;

struct gliderStateStruct
{
    uint8_t gliderDepth: 3; // value from 0 to 5
    uint8_t gliderInclination: 2; // 0 = horizontal, 1 = left, 2 = right
};
typedef struct gliderStateStruct gliderStateStruct;

struct ringsStatesStruct
{
    uint8_t x;
    uint8_t y;
    uint8_t step: 4;
    uint8_t empty: 1;
    // 2 bits left
};
typedef struct ringsStatesStruct ringsStatesStruct;

static gameStateStruct gameState;
static gliderStateStruct gliderState;

#define GLIDER_NUM_RINGS    8
static ringsStatesStruct ringStates[GLIDER_NUM_RINGS] = {
    {10,8,15}, {23,8,15}, {36,8,15}, {49,8,15},
    {10,20,15}, {23,20,15}, {36,20,15}, {49,20,15}
};


// TODO das sind alles TEMP variablen!!!
static uint8_t offsetX = 0;
static uint8_t offsetY = 0;
static uint8_t objDepth = 0;
static uint8_t groundOffset = 0;



static void initGame(uint8_t state)
{
    setScreen(BLACK);

    // game is starting
    if(state == 0)
    {
        gameState.gameOver = 0;
        gameState.paused = 0;
        gameState.nextLevel = 0;

        gliderState.gliderDepth = 2;
        gliderState.gliderInclination = 0;
    }
    else // game is continuing
    {
	}
}

static void handleInputs()
{
	// pause and unpause game
	if(isPushed(BUTTON_ZERO))
	{
        gameState.paused = !gameState.paused;
		checkForReset(); // see game.h
	}

    if(!gameState.paused)
	{
        // assume glider is not inclined, a button push may change this state
        gliderState.gliderInclination = 0;

        if(isPushed(BUTTON_LEFT))
        {
            offsetX--;
            gliderState.gliderInclination = 1;
        }
        if(isPushed(BUTTON_RIGHT))
        {
            offsetX++;
            gliderState.gliderInclination = 2;
        }
        if(isPushed(BUTTON_DOWN))
        {
            offsetY++;
        }
        if(isPushed(BUTTON_UP))
        {
            offsetY--;
        }
        if(isPushed(BUTTON_ONE))
        {
            if(gameState.show3d == 1 && gliderState.gliderDepth > 0) gliderState.gliderDepth--;
        }
        if(isPushed(BUTTON_TWO))
        {
            if(gameState.show3d == 1 && gliderState.gliderDepth < 4) gliderState.gliderDepth++;
        }
	}
}

static void paintGround()
{
    uint8_t yStart = 27;
    // determine colors
    uint8_t groundColor, groundLineColor;
    if(gameState.show3d)
    {
        groundColor = GLIDER_3D_GROUND_COLOR;
        groundLineColor = GLIDER_3D_COLOR_CENTER;
    }
    else
    {
        groundColor = GLIDER_GROUND_COLOR;
        groundLineColor = GLIDER_GROUND_LINE;
    }

    setCompleteHLine(yStart-1, groundLineColor);
    paintFilledRectangle(0, yStart, COLUMNS-1, LINES-1, groundColor);

    uint8_t counterGround = yStart + 1 + (groundOffset % 5);
    for(; counterGround < 42; counterGround+=5)
    {
        setCompleteHLine(counterGround, groundLineColor);
    }


    if(gameState.show3d)
    {
        // lines on left half of the screen
        drawLine(26, yStart, 20, LINES-1, GLIDER_3D_COLOR_RIGHT);
        drawLine(25, yStart, 19, LINES-1, GLIDER_3D_COLOR_CENTER);
        drawLine(21, yStart+11, 20, LINES-1, GLIDER_3D_COLOR_CENTER);

        drawLine(18, yStart,  8, LINES-1, GLIDER_3D_COLOR_RIGHT);
        drawLine(17, yStart,  7, LINES-1, GLIDER_3D_COLOR_CENTER);
        drawLine(10, yStart+11, 8, LINES-1, GLIDER_3D_COLOR_CENTER);

        drawLine(11, yStart, 0, LINES-4, GLIDER_3D_COLOR_RIGHT);
        drawLine(10, yStart, 0, LINES-5, GLIDER_3D_COLOR_CENTER);

        drawLine(4, yStart, 0, 30, GLIDER_3D_COLOR_RIGHT);
        drawLine(3, yStart, 0, 29, GLIDER_3D_COLOR_CENTER);


        // lines on right half of the screen
        drawLine(29, yStart, 35, LINES-1, GLIDER_3D_COLOR_LEFT);
        drawLine(30, yStart, 36, LINES-1, GLIDER_3D_COLOR_CENTER);
        drawLine(34, yStart+11, 35, LINES-1, GLIDER_3D_COLOR_CENTER);

        drawLine(37, yStart, 47, LINES-1, GLIDER_3D_COLOR_LEFT);
        drawLine(38, yStart, 48, LINES-1, GLIDER_3D_COLOR_CENTER);
        drawLine(45, yStart+11, 47, LINES-1, GLIDER_3D_COLOR_CENTER);

        drawLine(44, yStart, COLUMNS-1, LINES-4, GLIDER_3D_COLOR_LEFT);
        drawLine(45, yStart, COLUMNS-1, LINES-5, GLIDER_3D_COLOR_CENTER);

        drawLine(51, yStart, COLUMNS-1, 30, GLIDER_3D_COLOR_LEFT);
        drawLine(52, yStart, COLUMNS-1, 29, GLIDER_3D_COLOR_CENTER);
    }
    else
    {
        // lines on left half of the screen
        drawLine(24, yStart, 18, LINES-1, GLIDER_GROUND_LINE);
        drawLine(17, yStart,  8, LINES-1, GLIDER_GROUND_LINE);
        drawLine(10, yStart,  0, LINES-4, GLIDER_GROUND_LINE);
        drawLine(3, yStart,   0, 29, GLIDER_GROUND_LINE);

        // lines on right half of the screen
        drawLine(31, yStart, 37, LINES-1, GLIDER_GROUND_LINE);
        drawLine(38, yStart, 47, LINES-1, GLIDER_GROUND_LINE);
        drawLine(45, yStart, COLUMNS-1, LINES-4, GLIDER_GROUND_LINE);
        drawLine(52, yStart, COLUMNS-1, 29, GLIDER_GROUND_LINE);
    }
}

static void paintOneObject(uint8_t size, uint8_t x, uint8_t y)
{
    // determine color to paint the object
    uint8_t color;
    if(gameState.show3d)
    {

        if(gameState.paintCenterColor)
        {
            color = GLIDER_3D_COLOR_CENTER;
            gameState.paintCenterColor = 0;
        }
        else if(gameState.paintLeftColor)
        {
            color = GLIDER_3D_COLOR_LEFT;
        }
        else
        {
            color = GLIDER_3D_COLOR_RIGHT;
        }
        gameState.paintLeftColor = !gameState.paintLeftColor;
    }
    else
    {
        color = gameState.non3DRingColor;
    }

    switch(size) // paint rings in non-3d-mode and circles in 3d mode
    {
    case 1:
        setPixel(x, y, color);
        break;
    case 2:
        setHLine(x-1, x, y, color);
        setHLine(x-1, x, y+1, color);
        break;
    case 3:
        setHLine(x-1,x+1,y-1,color);
        setHLine(x-1,x+1,y+1,color);
        setPixel(x-1, y, color);
        setPixel(x+1, y, color);
        if(gameState.show3d) setPixel(x, y, color);
        break;
    case 4:
        setHLine(x-1,x,y-1,color);
        setHLine(x-1,x,y+2,color);
        setVLine(x-2,y,y+1,color);
        setVLine(x+1,y,y+1,color);
        if(gameState.show3d) paintFilledRectangle(x-1,y,x,y+1,color);
        break;
    case 5:
        setHLine(x-1,x+1,y-2,color);
        setHLine(x-1,x+1,y+2,color);
        setVLine(x-2,y-1,y+1,color);
        setVLine(x+2,y-1,y+1,color);
        if(gameState.show3d) paintFilledRectangle(x-1, y-1, x+1, y+1, color);
        break;
    case 6:
        setHLine(x-1,x+1,y-3,color);
        setHLine(x-1,x+1,y+3,color);
        setVLine(x-3,y-1,y+1,color);
        setVLine(x+3,y-1,y+1,color);
        setPixel(x-2,y-2,color);
        setPixel(x-2,y+2,color);
        setPixel(x+2,y-2,color);
        setPixel(x+2,y+2,color);
        if(gameState.show3d) paintFilledRectangle(x-2, y-2, x+2, y+2, color);
        break;
    }
}

static void paintObjects(uint8_t x, uint8_t y, uint8_t step)
{
    switch(step)
    {
    case 0:
    case 1:
        gameState.non3DRingColor = LIGHT_CYAN;
        paintOneObject(1, x, y);
        break;
    case 2:
    case 3:
        gameState.non3DRingColor = LIGHT_CYAN;
        paintOneObject(2, x, y);
        break;
    case 4:
    case 5:
        gameState.non3DRingColor = LIGHT_CYAN;
        paintOneObject(3, x, y);
        break;
    case 6:
        gameState.non3DRingColor = LIGHT_GREEN;
        paintOneObject(4, x, y);
        break;
    case 7:
        gameState.non3DRingColor = LIGHT_GREEN;
        paintOneObject(5, x, y);
        break;
    case 8:
    case 9:
        gameState.non3DRingColor = LIGHT_GREEN;
        paintOneObject(6, x, y);
        break;
    case 10: // center
    case 11:
        gameState.non3DRingColor = GREEN;
        paintOneObject(6, x, y);
        break;
    }
}

static void paintObjects3D(uint8_t x, uint8_t y, uint8_t step)
{
    // always start with the left color
    gameState.paintLeftColor = 1;

    switch(step)
    {
    case 0:
        paintOneObject(1, x-2, y);
        paintOneObject(1, x+2, y);
        break;
    case 1:
        paintOneObject(1, x-1, y);
        paintOneObject(1, x+2, y);
        break;
    case 2:
        paintOneObject(2, x-1, y);
        paintOneObject(2, x+3, y);
        break;
    case 3:
        paintOneObject(2, x-1, y);
        paintOneObject(2, x+2, y);
        break;
    case 4:
        paintOneObject(3, x-1, y);
        paintOneObject(3, x+1, y);
        setVLine(x,y-1,y+1,GLIDER_3D_COLOR_CENTER);
        break;
    case 5:
        paintOneObject(4, x-1, y);
        paintOneObject(4, x+2, y);
        setVLine(x,y,y+1,GLIDER_3D_COLOR_CENTER);
        break;
    case 6:
        paintOneObject(5, x-1, y);
        paintOneObject(5, x+1, y);
        setVLine(x-1,y-1,y+1,GLIDER_3D_COLOR_CENTER);
        setVLine(x,y-2,y+2,GLIDER_3D_COLOR_CENTER);
        setVLine(x+1,y-1,y+1,GLIDER_3D_COLOR_CENTER);
        break;
    case 7:
        paintOneObject(6, x-1, y);
        paintOneObject(6, x+2, y);
        paintFilledRectangle(x-1, y-1, x+2, y+1, GLIDER_3D_COLOR_CENTER);
        setHLine(x, x+1, y-2, GLIDER_3D_COLOR_CENTER);
        setHLine(x, x+1, y+2, GLIDER_3D_COLOR_CENTER);
        break;
    case 8:
        paintOneObject(6, x-1, y);
        paintOneObject(6, x+1, y);
        setPixel(x,y-3,GLIDER_3D_COLOR_CENTER);
        setPixel(x,y+3,GLIDER_3D_COLOR_CENTER);
        paintFilledRectangle(x-2, y-1, x+2, y+1, GLIDER_3D_COLOR_CENTER);
        setHLine(x-1, x+1, y-2, GLIDER_3D_COLOR_CENTER);
        setHLine(x-1, x+1, y+2, GLIDER_3D_COLOR_CENTER);
        break;
    case 9:
        gameState.paintCenterColor = 1;
        paintOneObject(6, x, y);
        break;
    case 10:
        paintOneObject(6, x+1, y);
        paintOneObject(6, x-1, y);
        setPixel(x,y-3,GLIDER_3D_COLOR_CENTER);
        setPixel(x,y+3,GLIDER_3D_COLOR_CENTER);
        paintFilledRectangle(x-2, y-1, x+2, y+1, GLIDER_3D_COLOR_CENTER);
        setHLine(x-1, x+1, y-2, GLIDER_3D_COLOR_CENTER);
        setHLine(x-1, x+1, y+2, GLIDER_3D_COLOR_CENTER);
        break;
    case 11:
        paintOneObject(6, x+2, y);
        paintOneObject(6, x-2, y);
        paintFilledRectangle(x-1, y-1, x+1, y+1, GLIDER_3D_COLOR_CENTER);
        setPixel(x, y-2, GLIDER_3D_COLOR_CENTER);
        setPixel(x, y+2, GLIDER_3D_COLOR_CENTER);
        break;
    }
}

static void paintGlider()
{
    uint8_t x, y;

    x = 10 + offsetX; // TODO temp vars
    y = 10+ offsetY;

    // determine color
    uint8_t gliderColor;
    if(gameState.show3d)
    {
        gliderColor = GLIDER_3D_COLOR_CENTER;
    }
    else
    {
        gliderColor = GLIDER_COLOR_CENTER;
    }


    if(gliderState.gliderInclination) // paint inclined glider
    {
        // in some cases this pixel should not be painted
        // it is saved to be repainted later
        uint8_t tempColor;
        if(gliderState.gliderInclination == 1) tempColor = getPixel(x-2, y+2);
        else tempColor = getPixel(x+2, y+2);

        // paint inclined glider
        uint8_t index;
        for(index = 4; index < 255; index--)
        {
            // left inclination
            if(gliderState.gliderInclination == 1) {
                setHLine(x-2, x-2+index, y+2-index, gliderColor); }
            // right inclination
            else {
                setHLine(x+2-index, x+2, y+2-index, gliderColor); }
        }

        switch(gliderState.gliderDepth)
        {
        case 4:
            if(gliderState.gliderInclination == 1)
            {
                setVLine(x-3, y-2, y+2, GLIDER_3D_COLOR_LEFT);
                setVLine(x-2, y-2, y+1, GLIDER_3D_COLOR_LEFT);
                drawLine(x-1, y+1, x+2, y-2, GLIDER_3D_COLOR_RIGHT);
                drawLine(x-1, y+2, x+3, y-2, GLIDER_3D_COLOR_RIGHT);
                setPixel(x-2, y+2, tempColor);
            }
            else
            {
                setVLine(x+3, y-2, y+2, GLIDER_3D_COLOR_RIGHT);
                setVLine(x+2, y-2, y+1, GLIDER_3D_COLOR_RIGHT);
                drawLine(x+1, y+1, x-2, y-2, GLIDER_3D_COLOR_LEFT);
                drawLine(x+1, y+2, x-3, y-2, GLIDER_3D_COLOR_LEFT);
                setPixel(x+2, y+2, tempColor);
            }
            break;
        case 3:
            if(gliderState.gliderInclination == 1)
            {
                setVLine(x-2, y-2, y+2, GLIDER_3D_COLOR_LEFT);
                drawLine(x-1, y+2, x+3, y-2, GLIDER_3D_COLOR_RIGHT);
            }
            else
            {
                setVLine(x+2, y-2, y+2, GLIDER_3D_COLOR_RIGHT);
                drawLine(x+1, y+2, x-3, y-2, GLIDER_3D_COLOR_LEFT);
            }
            break;
        case 2:
            // do nothing in this case: inclined glider is already painted
            break;
        case 1:
            if(gliderState.gliderInclination == 1)
            {
                setVLine(x-2, y-2, y+2, GLIDER_3D_COLOR_RIGHT);
                drawLine(x-1, y+2, x+3, y-2, GLIDER_3D_COLOR_LEFT);
            }
            else
            {
                setVLine(x+2, y-2, y+2, GLIDER_3D_COLOR_LEFT);
                drawLine(x+1, y+2, x-3, y-2, GLIDER_3D_COLOR_RIGHT);
            }
            break;
        case 0:
            if(gliderState.gliderInclination == 1)
            {
                setVLine(x-3, y-2, y+2, GLIDER_3D_COLOR_RIGHT);
                setVLine(x-2, y-2, y+1, GLIDER_3D_COLOR_RIGHT);
                drawLine(x-1, y+1, x+2, y-2, GLIDER_3D_COLOR_LEFT);
                drawLine(x-1, y+2, x+3, y-2, GLIDER_3D_COLOR_LEFT);
                setPixel(x-2, y+2, tempColor);
            }
            else
            {
                setVLine(x+3, y-2, y+2, GLIDER_3D_COLOR_LEFT);
                setVLine(x+2, y-2, y+1, GLIDER_3D_COLOR_LEFT);
                drawLine(x+1, y+1, x-2, y-2, GLIDER_3D_COLOR_RIGHT);
                drawLine(x+1, y+2, x-3, y-2, GLIDER_3D_COLOR_RIGHT);
                setPixel(x+2, y+2, tempColor);
            }
            break;
        }
    }
    else // paint horizontal glider
    {        
        // in some cases this pixel should not be painted
        // it is saved to be repainted later
        uint8_t tempColor = getPixel(x, y-2);

        // paint horizontal glider
        uint8_t cnt;
        for(cnt = 0; cnt < 4; cnt++)
        {
            setHLine(x-cnt, x+cnt, y-2+cnt, gliderColor);
        }

        switch(gliderState.gliderDepth)
        {
        case 4:
            drawLine(x-4, y+1, x-1, y-2, GLIDER_3D_COLOR_LEFT);
            drawLine(x-3, y+1, x-1, y-1, GLIDER_3D_COLOR_LEFT);
            drawLine(x+1, y-2, x+4, y+1, GLIDER_3D_COLOR_RIGHT);
            drawLine(x+1, y-1, x+3, y+1, GLIDER_3D_COLOR_RIGHT);
            setPixel(x, y-2, tempColor);
            break;
        case 3:
            drawLine(x-3, y+1, x, y-2, GLIDER_3D_COLOR_LEFT);
            drawLine(x+1, y-2, x+4, y+1, GLIDER_3D_COLOR_RIGHT);
            break;
        case 2:
            // do nothing in this case: horizontal glider is already painted
            break;
        case 1:
            drawLine(x-3, y+1, x, y-2, GLIDER_3D_COLOR_RIGHT);
            drawLine(x+1, y-2, x+4, y+1, GLIDER_3D_COLOR_LEFT);
            break;
        case 0:
            drawLine(x-4, y+1, x-1, y-2, GLIDER_3D_COLOR_RIGHT);
            drawLine(x-3, y+1, x-1, y-1, GLIDER_3D_COLOR_RIGHT);
            drawLine(x+1, y-2, x+4, y+1, GLIDER_3D_COLOR_LEFT);
            drawLine(x+1, y-1, x+3, y+1, GLIDER_3D_COLOR_LEFT);
            setPixel(x, y-2, tempColor);
            break;
        }
    }
}

static void repaint()
{
    wait(120);

    if(gameState.show3d)
    {
        setScreen(GLIDER_3D_BACKGROUND);
    }
    else
    {
        setScreen(GLIDER_BACKGROUND);
    }

    paintGround();

    if(gameState.show3d)
    {
        // paint all circles / rings
        uint8_t cnt;
        for(cnt = 0; cnt < GLIDER_NUM_RINGS; cnt++)
        {
            paintObjects3D(ringStates[cnt].x, ringStates[cnt].y, ringStates[cnt].step);
        }
    }
    else
    {
        // paint all circles / rings
        uint8_t cnt;
        for(cnt = 0; cnt < GLIDER_NUM_RINGS; cnt++)
        {
            paintObjects(ringStates[cnt].x, ringStates[cnt].y, ringStates[cnt].step);
        }
    }

    paintGlider();
}

static void updateRings()
{
    uint8_t ringCnt;
    for(ringCnt = 0; ringCnt <= GLIDER_NUM_RINGS; ringCnt++)
    {
        // update all ring positions
        if(ringStates[ringCnt].step < 15 && ringStates[ringCnt].empty != 1)
        {
            ringStates[ringCnt].step++;
        }
        else if(ringStates[ringCnt].step == 15)
        {
            ringStates[ringCnt].empty = 1;
        }

        // empty storage position found: create new object
        if(ringStates[ringCnt].empty)
        {
            // create object
            ringStates[ringCnt].empty = 0;
            ringStates[ringCnt].step = 0;
            break;
        }
    }
}

static void computeNextStep()
{
    // slows down calculation of next step
    if(update++ % UPDATE_LIMIT == 0)
    {
        if(update2++ == gameSpeed)
        {
            update2 = 0;
            groundOffset++;
            //objDepth++;

            updateRings();

            //if(objDepth == 15) objDepth = 0;
        }
    }
}

static void handleGameOver()
{
    // handle game over here
    writeString("GAME OVER", 8, 15, VIOLET);

    wait(500);
    // before return wait for button press
    while(!isPushed(ANY_BUTTON))
    {
        wait(0);// wait is only for the simulator
    }

    //showHighScoreMenu(score);
}


void glider_startGame()
{
    showGameTitle("GLIDER", MINT, BLACK, 15, 15);
	// set myOP1 to a value between 1 and 3, default 1 and set myOP2 to a value between 1 and 9, default 3
    {
        uint8_t mode3d;
        uint8_t op2; // TODO second option
        showOptionsMenu("3D__myOP2",14,15, &mode3d, 0, 1, 0, &op2, 1, 9, 3);
        gameState.show3d = mode3d;
    }

    initGame(0);
	while(1)
	{
		do
		{
			handleInputs();
		}
        while(gameState.paused);
		repaint();
		computeNextStep();
        if(gameState.nextLevel) initGame(1);
        if(gameState.gameOver) break;
		wait(1);// only for the simulator
	}
	handleGameOver();
}

