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
 

#ifndef GAME_H
#define GAME_H


#ifdef SIMULATOR
    // for the simulator
    #include "../MonstaDefs.h"
#else
    // for the microcontroller
    #include "../util.h"
#endif


/**
*	Shows the main menu for selecting games
*/
uint8_t showGameSelectMenu();

/**
*	Shows the high score menu. If the given score is one of the best, it is added to the list of highscores
*/
void showHighScoreMenu(uint16_t score);


/**
*	Starts the game on position gameNumber in the games list
*/
void loadGame(uint8_t gameNumber);

/**
*	Prints the game title in the color stringColor an a screen with the color backgroundColor
*	with the string's upper left cornet at position (x,y)
*/
void showGameTitle(char* title, uint8_t stringColor, uint8_t backgroundColor, uint8_t x, uint8_t y);

/**
*	Check whether the reset condition is given and if so, resets the MC
*	The reset condition occurs whenever BUTTON_ZERO (yellow) is pushed and held for approx. 2 seconds
*/
void checkForReset();

/**
*	Displays the options menu with the two options given in the optionsText. The options should be separated
*	by two line breaks (example: "LEVEL__SPEED"). The text is displayed at the position xPos and yPos.
*	firstOption and secondOption are pointers to the variables that shall be set in the options menu.
*	They will be set to a value between minValFirst(minValSecond) (inclusive) and maxValFirst(maxValSecond) (inclusive)
*	When the options menu is displayed, the variables are initialized with the values stdValFirst(stdValSecond)
*/
void showOptionsMenu(char* optionsText, uint8_t xPos, uint8_t yPos,
			uint8_t* firstOption, uint8_t minValFirst, uint8_t maxValFirst, uint8_t stdValFirst,
			uint8_t* secondOption, uint8_t minValSecond, uint8_t maxValSecond, uint8_t stdValSecond);


#endif
