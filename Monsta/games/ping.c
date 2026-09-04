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
#include "ping.h"

#define RACKET_SIZE 	5 // real size is this plus one
#define PLAYER_COLOR	GRASS_GREEN
#define CPU_COLOR		RED

#define SLOW_UPDATE_LIMIT	128
#define FAST_UPDATE_LIMIT	64

// boundaries for the racket position
static uint8_t racket1MaxVal = LINES-5-RACKET_SIZE;
static uint8_t racket2MaxVal = COLUMNS-11-RACKET_SIZE;

static uint8_t paused = 0;
static uint8_t gameOver;
static uint16_t update = 0; // used to slow down calculation of the next step
static uint16_t updateLimit; // determines speed of next step calculation

// upper edge of vertical rackets
static uint8_t racketPlayer1;
static uint8_t racketCPU1;
// left enge of horizontal rackets
static uint8_t racketPlayer2;
static uint8_t racketCPU2;

static Point2D ballPos;
static Point2D ballDir; // valid values: 0, 1, 255 (i.e. -1)
static uint8_t ballColor;

static uint8_t scorePlayer;
static uint8_t scoreCPU;

static uint8_t maxScore;
static uint8_t gameLevel;
// the player can perform a super shot
static uint8_t superShot = 0;

static void initBall(uint8_t side)
{
    ballColor = WHITE;
    updateLimit = SLOW_UPDATE_LIMIT;

    // random ball direction
    uint8_t ranDir = (getRandomNumber() % 2 == 0) ? 1 : 255;
    // random ball position
    uint8_t ranPos = getRandomNumber() % 3;
    ranPos -= 1;

    if(side == 1) // left side
    {
        ballPos.x = 5 + ranPos;
        ballPos.y = 18;
        ballDir.x = 1;
        ballDir.y = ranDir; //1;
    }
    else if(side == 2) // right side
    {
        ballPos.x = COLUMNS-6  + ranPos;
        ballPos.y = 18;
        ballDir.x = 255;
        ballDir.y = ranDir; //255;
    }
    else if(side == 3) // upper side
    {
        ballPos.x = 28;
        ballPos.y = 5  + ranPos;
        ballDir.x = ranDir; //255;
        ballDir.y = 1;
    }
    else // side == 4, lower side
    {
        ballPos.x = 28;
        ballPos.y = LINES-9  + ranPos;
        ballDir.x = ranDir; //1;
        ballDir.y = 255;
    }
}

static void initGame()
{
    setScreen(BLACK);
    initBall(1);

    gameOver = 0;
    scorePlayer = 0;
    scoreCPU = 0;

    racketPlayer1 = 16;
    racketCPU1 = 16;
    racketPlayer2 = 26;
    racketCPU2 = 26;

    // paint default game field borders
    setCompleteHLine(0, BLUE);
    setCompleteHLine(LINES-4,BLUE);

    // modify default borders for levels 2 and 3
    if(gameLevel > 1)
    {
        setHLine(10,COLUMNS-11,0, BLACK);
        setHLine(10,COLUMNS-11,LINES-4, BLACK);
    }


    // paint default rackets (vertical)
    setVLine(0,racketPlayer1,racketPlayer1+RACKET_SIZE,PLAYER_COLOR);
    setVLine(COLUMNS-1,racketCPU1,racketCPU1+RACKET_SIZE,CPU_COLOR);

    // modify default vertical rackets for level 2 (both vertical rackets are for the player)
    if(gameLevel == 2)
    {
        setVLine(COLUMNS-1,racketPlayer1,racketPlayer1+RACKET_SIZE,PLAYER_COLOR);
    }

    // add horizontal rackets for levels 2 and 3
    if(gameLevel > 1)
    {
        uint8_t tempColorUpper = CPU_COLOR;
        uint8_t tempColorLower = PLAYER_COLOR;

        // in level 2 both horizontal rackets are for the CPU
        if(gameLevel == 2) tempColorLower = CPU_COLOR;

        setHLine(racketCPU2,racketCPU2+RACKET_SIZE,0,tempColorUpper);
        setHLine(racketPlayer2,racketPlayer2+RACKET_SIZE,LINES-4,tempColorLower);
    }
}


static void paintScore()
{
    if(scorePlayer > 0) setPixel(scorePlayer*2-1,LINES-2,PLAYER_COLOR);
    if(scoreCPU > 0) setPixel(COLUMNS-1-(scoreCPU*2-1),LINES-2,CPU_COLOR);
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
        // player move racket up and repaint
        if(isPushed(BUTTON_UP))
        {
            if(racketPlayer1 >= 2)
            {
                setPixel(0,racketPlayer1+RACKET_SIZE,BLACK);
                racketPlayer1 -= 1;
                setPixel(0,racketPlayer1,PLAYER_COLOR);

                // move second vertical racket
                if(gameLevel == 2)
                {
                    racketPlayer1 += 1;
                    setPixel(COLUMNS-1,racketPlayer1+RACKET_SIZE,BLACK);
                    racketPlayer1 -= 1;
                    setPixel(COLUMNS-1,racketPlayer1,PLAYER_COLOR);
                }
            }
        }
        // player move racket down and repaint
        if(isPushed(BUTTON_DOWN))
        {
            if(racketPlayer1 < racket1MaxVal)
            {
                setPixel(0,racketPlayer1,BLACK);
                racketPlayer1 += 1;
                setPixel(0,racketPlayer1+RACKET_SIZE,PLAYER_COLOR);

                // move second vertical racket
                if(gameLevel == 2)
                {
                    racketPlayer1 -= 1;
                    setPixel(COLUMNS-1,racketPlayer1,BLACK);
                    racketPlayer1 += 1;
                    setPixel(COLUMNS-1,racketPlayer1+RACKET_SIZE,PLAYER_COLOR);
                }
            }
        }
        // player move racket left and repaint
        if(isPushed(BUTTON_LEFT))
        {
            if(gameLevel == 3 && racketPlayer2 >= 11)
            {
                setPixel(racketPlayer2+RACKET_SIZE,LINES-4,BLACK);
                racketPlayer2 -= 1;
                setPixel(racketPlayer2,LINES-4,PLAYER_COLOR);
            }
        }
        // player move racket right and repaint
        if(isPushed(BUTTON_RIGHT))
        {
            if(gameLevel == 3 && racketPlayer2 < racket2MaxVal)
            {
                setPixel(racketPlayer2,LINES-4,BLACK);
                racketPlayer2 += 1;
                setPixel(racketPlayer2+RACKET_SIZE,LINES-4,PLAYER_COLOR);
            }
        }
        // perform superShot if ball hits racket at the center and button is pushed
        if(isPushed(BUTTON_ONE) || isPushed(BUTTON_TWO))
        {
            // super shot in every level
            if(ballPos.x == 1 && (racketPlayer1+2 == ballPos.y || racketPlayer1+3 == ballPos.y))
            {
                superShot = 1;
            }
            // super shot in level 2 only
            else if(gameLevel == 2 && ballPos.x == COLUMNS-2 && (racketPlayer1+2 == ballPos.y || racketPlayer1+3 == ballPos.y))
            {
                superShot = 2;
            }
            // super shot in level 3 only
            else if(gameLevel == 3 && (ballPos.x == racketPlayer2+2 || ballPos.x == racketPlayer2+3) && ballPos.y == LINES-5)
            {
                superShot = 3;
            }
        }
    }
}

static void computeNextStep()
{
    // slows down calculation of next step
    update++;

    if(update % SLOW_UPDATE_LIMIT == 0)
    {
        // --- CPU make move ---
        // computers also make mistakes: if the time counter % 16 is 0, than no move is made
        if(getTimestamp() % 16 != 0)
        {
            if(gameLevel != 2)
            {
                if(racketCPU1 >= 2 && racketCPU1+2 > ballPos.y)
                {
                    // move racket up
                    setPixel(COLUMNS-1,racketCPU1+RACKET_SIZE,BLACK);
                    racketCPU1 -= 1;
                    setPixel(COLUMNS-1,racketCPU1,CPU_COLOR);
                }
                else if(racketCPU1 < racket1MaxVal && racketCPU1 + RACKET_SIZE-2 < ballPos.y)
                {
                    // move racket down
                    setPixel(COLUMNS-1,racketCPU1,BLACK);
                    racketCPU1 += 1;
                    setPixel(COLUMNS-1,racketCPU1+RACKET_SIZE,CPU_COLOR);
                }
            }
            if(gameLevel > 1)
            {
                if(racketCPU2 >= 11 && racketCPU2+2 > ballPos.x)
                {
                    // move racket left
                    setPixel(racketCPU2+RACKET_SIZE,0,BLACK);
                    racketCPU2 -= 1;
                    setPixel(racketCPU2,0,CPU_COLOR);

                    if(gameLevel == 2) // second racket in gameLevel 2
                    {
                        racketCPU2 +=1;
                        setPixel(racketCPU2+RACKET_SIZE,LINES-4,BLACK);
                        racketCPU2 -= 1;
                        setPixel(racketCPU2,LINES-4,CPU_COLOR);
                    }
                }
                else if(racketCPU2 < racket2MaxVal && racketCPU2 + RACKET_SIZE-2 < ballPos.x)
                {
                    // move racket right
                    setPixel(racketCPU2,0,BLACK);
                    racketCPU2 += 1;
                    setPixel(racketCPU2+RACKET_SIZE,0,CPU_COLOR);

                    if(gameLevel == 2) // second racket in gameLevel 2
                    {
                        racketCPU2 -=1;
                        setPixel(racketCPU2,LINES-4,BLACK);
                        racketCPU2 += 1;
                        setPixel(racketCPU2+RACKET_SIZE,LINES-4,CPU_COLOR);
                    }
                }
            }
        }
    }

    // updateLimit is normally 256 or in superShot: 128
    if(update % updateLimit == 0)
    {
        // ---- check next ball position ----
        uint8_t nextXPosColor = getPixel(ballPos.x+ballDir.x,ballPos.y);
        uint8_t nextYPosColor = getPixel(ballPos.x,ballPos.y+ballDir.y);
        uint8_t nextDiagPosColor = getPixel(ballPos.x+ballDir.x,ballPos.y+ballDir.y);

        // if next pixel is an obstacle, change direction (255 is for -1)
        if(nextXPosColor != BLACK)
        {
            ballDir.x = ~ballDir.x + 1;
            // if(ballDir.x == 1) ballDir.x = 255;
            // else if(ballDir.x == 255) ballDir.x = 1;
            ballColor = WHITE;
            updateLimit = SLOW_UPDATE_LIMIT;
            playSong(SHORT_HIGH_BEEP);
        }
        if(nextYPosColor != BLACK)
        {
            ballDir.y = ~ballDir.y + 1;
            //if(ballDir.y == 1) ballDir.y = 255;
            //else if(ballDir.y == 255) ballDir.y = 1;
            ballColor = WHITE;
            updateLimit = SLOW_UPDATE_LIMIT;
            playSong(SHORT_HIGH_BEEP);
        }
        // handle obstacles in the diagonal
        if(nextDiagPosColor != BLACK && nextXPosColor == BLACK && nextYPosColor == BLACK)
        {
            ballDir.x = ~ballDir.x + 1;
            ballDir.y = ~ballDir.y + 1;
            ballColor = WHITE;
            updateLimit = SLOW_UPDATE_LIMIT;
            playSong(SHORT_HIGH_BEEP);
        }

        // delete ball on old position
        setPixel(ballPos.x, ballPos.y, BLACK);
	
        // handle super shot
        if(superShot)
        {
            ballColor = ORANGE;
			uint8_t extra = getTimestamp()%32;
            updateLimit = FAST_UPDATE_LIMIT+extra;
            superShot = 0;
        }

        // update ball position
        ballPos.x += ballDir.x;
        ballPos.y += ballDir.y;

        // paint ball on new position
        setPixel(ballPos.x, ballPos.y, ballColor);


        // --- check whether someone scored ---
        uint8_t scored = 0;
        // next pixel is a goal
        if(ballPos.x == 0 || (gameLevel == 3 && ballPos.y == LINES-4) || (gameLevel == 2 && ballPos.x == COLUMNS-1))
        {
            scoreCPU++;
            // scored indicated the edge where the ball ended
            if(ballPos.x == 0) scored = 1;
            else if(ballPos.x == COLUMNS-1) scored = 2;
            else scored = 4; //ballPos.y == LINES-4
        }
        else if(ballPos.y == 0 || (gameLevel != 2 && ballPos.x == COLUMNS-1) || (gameLevel == 2 && ballPos.y == LINES-4))
        {
            scorePlayer++;
            // scored indicated the edge where the ball ended
            if(ballPos.y == 0) scored = 3;
            else if(ballPos.x == COLUMNS-1) scored = 2;
            else scored = 4; //ballPos.y == LINES-4
        }

        if(scored)
        {
            wait(1000);
            setPixel(ballPos.x, ballPos.y, BLACK);
            initBall(scored);
            setPixel(ballPos.x, ballPos.y, ballColor);
            paintScore();
            wait(1000);
            if(scorePlayer == maxScore || scoreCPU == maxScore) gameOver = 1;
        }
    }
}

static void handleGameOver()
{
    setScreen(BLACK);
    if(scorePlayer > scoreCPU)
    {
        writeString("YOU WIN!",12,15, PLAYER_COLOR);
    }
    else
    {
        writeString("YOU LOSE...",10,15, CPU_COLOR);
    }
    while(!isPushed(ANY_BUTTON))
    {
        wait(0);// only for the simulator
    }
    wait(200);
}


void ping_startGame()
{
    showGameTitle("PING", MINT, BLACK, 19, 15);
    showOptionsMenu("LEVEL__SCORE",14,15, &gameLevel, 1, 3, 1, &maxScore, 1, 9, 3);
    initGame();
    while(1)
    {
        do
        {
            handleInputs();
        }
        while(paused);
        computeNextStep();
        if(gameOver) break;
        wait(1);// only for the simulator
    }
    handleGameOver();
}

