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
#include "feedme.h"

#define FEEDME_BACKGROUND   CYAN
#define UPDATE_LIMIT        32
static uint8_t gameSpeed; // used to slow down or increase game speed (high values = slow)

// monsta changes color when a life is lost
#define FIRST_MONSTA_COLOR      MINT
#define SECOND_MONSTA_COLOR     LIGHT_RED
#define THIRD_MONSTA_COLOR      PINK
#define FOURTH_MONSTA_COLOR     RED
static uint8_t currentMonstaColor;

#define FOOD_POSITIONS      25
#define MOUTH_POSITION      20 // index of position of the mouth
#define FOOD_POINTS         13

static uint8_t paused = 0;
static uint8_t gameOver;
static uint8_t lifes;
static uint8_t redFruitEaten;
static uint8_t bonusFruit;
static uint16_t score, oldScore;

static uint8_t update = 0; // used to slow down calculation of the next step
static uint8_t update2 = 0;

static uint8_t mouthOpen;
static uint8_t repaintMouth;
static uint8_t mouthX = 13; // position of the upper left mouth corner
static uint8_t mouthY = 26;
static uint16_t mouthClosedTime;
static uint16_t gameStartedTime;


// index 0 means: invalid position
static Point2D foodPositions[FOOD_POSITIONS] = {
    {0,0}, {54,15}, {52,12}, {50,9}, {48,7}, {46,5}, {43,3}, {40,2}, {37,1}, // rising from the right
    {34,1}, {31,2}, {28,3}, {25,5}, {23,7}, {21,9}, {20,12}, {19,15}, {18,18}, {17,21}, {16,24}, {15,27}, // falling to the left
    {20,27}, {23,30}, {26,33}, {28,38} // falling away
};

struct FoodPoint
{
    uint8_t positionIndex;
    uint8_t oldPositionIndex;
    uint8_t color;
};
typedef struct FoodPoint FoodPoint;

static FoodPoint foodPoints[FOOD_POINTS] = {
    {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}
};


static void resetFruitPositions()
{
    // set all position indices to zero
    uint8_t i;
    for(i = 0; i < FOOD_POINTS; i++)
    {
        foodPoints[i].positionIndex = 0;
        foodPoints[i].oldPositionIndex = 0;
    }

    uint8_t tmpX, tmpY;
    // delete all fruit on the screen
    for(i = 0; i < FOOD_POSITIONS; i++)
    {
        tmpX = foodPositions[i].x;
        tmpY = foodPositions[i].y;
        // paint food point
        paintFilledRectangle(tmpX,tmpY,tmpX+1,tmpY+1,FEEDME_BACKGROUND);
    }
}


static void paintOpenMouth()
{
    paintFilledRectangle(mouthX,mouthY,mouthX+4,mouthY+4,FEEDME_BACKGROUND);
    setVLine(mouthX, mouthY, mouthY+4, BLACK);
    setHLine(mouthX, mouthX+4, mouthY+4, BLACK);
}

static void paintClosedMouth()
{
    paintFilledRectangle(mouthX,mouthY,mouthX+4,mouthY+4,currentMonstaColor);
    setPixel(mouthX+3, mouthY, FEEDME_BACKGROUND);
    setPixel(mouthX+4, mouthY, FEEDME_BACKGROUND);
    setPixel(mouthX+4, mouthY+1, FEEDME_BACKGROUND);

    uint8_t i;
    for(i = 0; i < 4; i++)
    {
        setPixel(mouthX+i, mouthY+4-i, BLACK);
    }
}

static void paintCrossEye()
{
    setPixel(mouthX-2,mouthY+2,WHITE);
    setPixel(mouthX-3,mouthY+4,BLACK);
    setPixel(mouthX-4,mouthY+3,BLACK);
}

static void paintMonsta()
{
    // body
    writeBitPattern(0x3BE31880, 5, 6, mouthX-5, mouthY, currentMonstaColor);
    setHLine(mouthX-5,mouthX+4,mouthY+5,currentMonstaColor);
    setHLine(mouthX-5,mouthX+4,mouthY+6,currentMonstaColor);
    setHLine(mouthX-4,mouthX+3,mouthY+7,currentMonstaColor);
    setHLine(mouthX-2,mouthX+1,mouthY+8,currentMonstaColor);

    //if(currentMonstaColor != FIRST_MONSTA_COLOR) return;
    // eye
    uint8_t i;
    for(i = 0; i < 3; i++)
    {
        setHLine(mouthX-4, mouthX-2, mouthY+2+i, WHITE);
        if(i)
        {
            setPixel(mouthX-3,mouthY+1+i,BLACK);
            setPixel(mouthX-2,mouthY+1+i,BLACK);
        }
    }
    // feet
    writeBitPattern(0x42A50000, 8, 2, mouthX-4, mouthY+8, BLACK);
}

static void initGame()
{
    setScreen(FEEDME_BACKGROUND);

    gameOver = 0;
    gameSpeed = 20;
    redFruitEaten = 0;
    bonusFruit = 0;
    lifes = 3;
    mouthOpen = 1;
    repaintMouth = 1;

    score = 0;
    oldScore = 1; // init with a different value than score

    resetFruitPositions();
    currentMonstaColor = FIRST_MONSTA_COLOR;

    // paint monsta
    paintMonsta();
    paintOpenMouth();
    // paint grass
    uint8_t i;
    for(i = 36; i < 42; i++)
    {
        setHLine(0,25,i,FRESH);
    }

    gameStartedTime = getTimestamp();
}

static void handleInputs()
{
    // pause and unpause game
    if(isPushed(BUTTON_ZERO))
    {
        paused = !paused;
        checkForReset(); // see game.h
    }

    if(!paused)
    {
        // open mouth
        if(isPushed(BUTTON_ONE) || isPushed(BUTTON_TWO) || isPushed(BUTTON_DOWN))
        {
            paintClosedMouth();
            mouthOpen = 0;
            repaintMouth = 1;
            mouthClosedTime = getTimestamp();
        }
    }
}

static void repaint()
{
    // paint closed mouth
    if(repaintMouth && getTimestamp() - mouthClosedTime > 5)
    {
        paintOpenMouth();
        mouthOpen = 1;
        repaintMouth = 0;
    }

    // paint score
    if(oldScore != score)
    {
        // create strings from score integers
        uint8_t lengthScore = (score < 1000) ? 4 : ( (score < 10000) ? 5 : 6);
        uint8_t lengthOldScore = (oldScore < 1000) ? 4 : ( (oldScore < 10000) ? 5 : 6);

        char oldScoreString[lengthOldScore];
        convertToString(oldScore,oldScoreString,lengthOldScore);
        char scoreString[lengthScore];
        convertToString(score,scoreString,lengthScore);

        writeString(oldScoreString,2,2,FEEDME_BACKGROUND); // delete old score
        writeString(scoreString,2,2,WHITE); // paint new score
        oldScore = score;
    }
}


static void setNextPosition(FoodPoint* food)
{
    // food.positionIndex should be greater 0
    uint8_t posIndex = (*food).positionIndex;
    if(posIndex > 0)
    {
        (*food).oldPositionIndex = (*food).positionIndex;

        // --- handle special cases ---
        if(posIndex == MOUTH_POSITION-1)
        {
            if(mouthOpen)
            {
                // next step: fruit is in the mouth
                (*food).positionIndex++;
				playSong(SHORT_VERY_HIGH_BEEP);
            }
            else
            {
                // next step: fruit is falling away
                (*food).positionIndex +=2;
				playSong(SHORT_HIGH_BEEP);
            }
        }
        else if(posIndex == MOUTH_POSITION)
        {
            // fruit has been eaten
            (*food).positionIndex = 0;
        }
        else if(posIndex == FOOD_POSITIONS-1)
        {
            // fruit has reached the end
            (*food).positionIndex = 0;
        }
        else
        {
            // all other cases: next position
            (*food).positionIndex++;
        }
    }

}

static void computeNextStep()
{
    // slows down calculation of next step
    if(update++ % UPDATE_LIMIT == 0)
    {
        // next step only computed every gameSpeed-th time the update reaches UPDATE_LIMIT
        if(update2++ == gameSpeed)
        {
            update2 = 0;
            // increase speed after 30 seconds (value: 750)
            if(getTimestamp() - gameStartedTime > 750)
            {
                // full speed after 7 minutes
                if(gameSpeed > 10) gameSpeed--; // low values = faster game
                gameStartedTime = getTimestamp();
            }


            // set to next position and add new food points (only those with position index == 0 are unused)
            uint8_t cnt, posIndex, newPointAdded = 0;
            for(cnt = 0; cnt < FOOD_POINTS; cnt++)
            {
                posIndex = foodPoints[cnt].positionIndex;

                // only initialize one food point at a time
                if(posIndex == 0 && !newPointAdded)
                {
                    uint8_t type = getRandomNumber() % 16;
                    uint8_t newColor;

                    switch (type)
                    {
                    case 0:
                    case 1:
                    case 2: newColor = RED; break;

                    case 3:
                    case 4:
                    case 5:
                    case 6: newColor = MINT;break;

                    case 7: newColor = LIGHT_YELLOW;break;

                    default: newColor = FEEDME_BACKGROUND;
                    }

                    // bonus fruits
                    if((bonusFruit == 0 && score >= 200)
                       || (bonusFruit == 1 && score >= 500)
                       || (bonusFruit == 2 && score >= 1000))
                    {
                        newColor = DARK_BLUE;
                        bonusFruit++;
                    }

                    if(newColor != FEEDME_BACKGROUND)
                    {
                        foodPoints[cnt].positionIndex = 1;
                        foodPoints[cnt].oldPositionIndex = 0;
                        foodPoints[cnt].color = newColor;
                    }
                    newPointAdded = 1;
                }
                // set next position
                else if(posIndex != 0)
                {
                    setNextPosition(&foodPoints[cnt]);
                }
            }

            // delete fruit points on old positions
            for(cnt = 0; cnt < FOOD_POINTS; cnt++)
            {
                posIndex = foodPoints[cnt].oldPositionIndex;
                if(posIndex != 0)
                {
                    uint8_t tmpX, tmpY;
                    if(posIndex != MOUTH_POSITION || (posIndex == MOUTH_POSITION && mouthOpen))
                    {
                        tmpX = foodPositions[posIndex].x;
                        tmpY = foodPositions[posIndex].y;
                        // delete food point on old position
                        paintFilledRectangle(tmpX,tmpY,tmpX+1,tmpY+1,FEEDME_BACKGROUND);
                    }
                }
            }

            // paint new positions
            uint8_t paintColor;
            for(cnt = 0; cnt < FOOD_POINTS; cnt++)
            {
                // only paint valid positions
                posIndex = foodPoints[cnt].positionIndex;
                if(posIndex > 0 && posIndex < FOOD_POSITIONS)
                {
                    uint8_t tmpX, tmpY;
                    paintColor = foodPoints[cnt].color;
                    tmpX = foodPositions[posIndex].x;
                    tmpY = foodPositions[posIndex].y;
                    // paint food point on new position
                    paintFilledRectangle(tmpX,tmpY,tmpX+1,tmpY+1,paintColor);
                }
            }


            // check whether one food point is in the mouth
            for(cnt = 0; cnt < FOOD_POINTS; cnt++)
            {
                if(foodPoints[cnt].positionIndex == MOUTH_POSITION)
                {
                    oldScore = score;
                    // check which food point has been eaten
                    switch (foodPoints[cnt].color)
                    {
                    case RED: score += 0; redFruitEaten = 1; break;
                    case MINT: score += 1; break;
                    case LIGHT_YELLOW: score += 5; break;
                        // bonus fruit resets lifes
                    case DARK_BLUE: score += 10;
                        lifes = 3;
                        currentMonstaColor = FIRST_MONSTA_COLOR;
                        paintMonsta();
                        gameSpeed += 5;
                        break;
                    }
                }
            }

            if(redFruitEaten)
            {
				playSong(ZONK);
                redFruitEaten = 0;
                gameSpeed += 2; // decrease game speed
                gameStartedTime = getTimestamp();

                lifes--; // lose one life
                switch (lifes)
                {
                case 2: currentMonstaColor = SECOND_MONSTA_COLOR; break;
                case 1: currentMonstaColor = THIRD_MONSTA_COLOR; break;
                case 0: currentMonstaColor = FOURTH_MONSTA_COLOR; break;
                }
                paintMonsta();
                paintCrossEye();
                wait(1000);

                paintMonsta();
                resetFruitPositions();
                if(lifes == 0)
                {
                    gameOver = 1;
                    paintCrossEye();
                    return;
                }
            }
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

    showHighScoreMenu(score);
}


void feedme_startGame()
{
    showGameTitle("FEEDME", MINT, BLACK, 15, 15);
    // set myOP1 to a value between 1 and 3, default 1 and set myOP2 to a value between 1 and 9, default 3
    //showOptionsMenu("LEVEL__EMPTY",14,15, &lifes, 1, 9, 1, &lifes, 0, 0, 0);
    initGame();
    while(1)
    {
        do
        {
            handleInputs();
        }
        while(paused);
        repaint();
        computeNextStep();
        if(gameOver) break;
        wait(1);// only for the simulator
    }
    handleGameOver();
}

