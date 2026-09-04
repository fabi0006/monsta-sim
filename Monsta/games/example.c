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
#include "example.h"
#include "stdbool.h"

#define MY_GAME_UPDATE_LIMIT	32 // for efficient computation recommended to be power of 2

struct gameStateStruct
{
    uint8_t paused: 1;
    uint8_t gameOver: 1;
    uint8_t nextLevel: 1;
};
typedef struct gameStateStruct gameStateStruct;

static gameStateStruct gameState;

static uint8_t update = 0; // used to slow down calculation of the next step: outer loop
static uint8_t update2 = 0;// ... : inner loop
static uint8_t gameSpeed = 100; // used to slow down or increase game speed (high values = slow)

// position des pixels
static Point2D pix;
static Point2D pix_old;

static bool up_pushed;
static bool down_pushed;
static bool right_pushed;
static bool left_pushed;

static void initGame(uint8_t state)
{
    // game is starting
    if(state == 0)
    {
        gameState.gameOver = 0;
        gameState.paused = 0;
	    gameState.nextLevel = 0;

        setScreen(BLACK);
        pix.x = 5;
        pix.y = 10;

        setVLine(30, 20, 40, MINT);

        up_pushed = down_pushed = right_pushed = left_pushed = false;
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
        up_pushed = down_pushed = right_pushed = left_pushed = false;

        if(isPushed(BUTTON_RIGHT))
        {
            right_pushed = true;
        }
        if(isPushed(BUTTON_LEFT))
        {
            left_pushed = true;
        }
        if(isPushed(BUTTON_UP))
        {
            up_pushed = true;
        }
        if(isPushed(BUTTON_DOWN))
        {
            down_pushed = true;
        }
	}
}

static void repaint()
{
    setVLine(30, 20, 40, MINT);

    if(pix_old.x != pix.x || pix_old.y != pix.y)
    {
        setPixel(pix_old.x, pix_old.y, BLACK);
    }
    setPixel(pix.x, pix.y, RED);
}

static void computeNextStep()
{
    // slows down calculation of next step
    if(update++ % MY_GAME_UPDATE_LIMIT == 0)
    {
        if(update2++ == gameSpeed)
        {
            update2 = 0;
			// calculate next step here

            if(left_pushed && pix.x > 1) // todo
            {
                pix.x -= 1;
            }
            if(right_pushed && pix.x < COLUMNS-2)
            {
                pix.x += 1;
            }
            if(up_pushed && pix.y > 1) // todo
            {
                 pix.y -= 1;
            }
            if(down_pushed && pix.y < LINES-2)
            {
                pix.y += 1;
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

    //showHighScoreMenu(score);
}


void example_startGame()
{
    showGameTitle("EXAMPLE", MINT, BLACK, 18, 15);
	// set myOP1 to a value between 1 and 3, default 1 and set myOP2 to a value between 1 and 9, default 3
    //showOptionsMenu("myOP1__myOP2",14,15, &myOP1, 1, 3, 1, &myOP2, 1, 9, 3);
    initGame(0);
	while(1)
	{
		do
		{
			handleInputs();
		}
        while(gameState.paused);
        computeNextStep();
        repaint();
        if(gameState.nextLevel) initGame(1);
        if(gameState.gameOver) break;
        wait(1);// only for the simulator
	}
	handleGameOver();
}

