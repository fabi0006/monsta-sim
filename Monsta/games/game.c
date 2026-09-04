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

// New Game Step 1: include header file
#include "ping.h"
#include "feedme.h"
#include "spaceFlight.h"
#include "amazingMaze.h"
#include "glider.h"


uint8_t mResetCounter = 0;
uint16_t mResetControlTime = 0;


uint8_t showGameSelectMenu(){

#define GAME_LIST_LENGTH 	5		// New Game Step 2: add one to GAME_LIST_LENGTH
#define SCREEN_LENGTH		5		// number of lines on the screen
#define SELECT_COLOR		MINT
#define MENU_COLOR		BLUE
#define BACKGROUND		WHITE

	// New Game Step 3: add name of game to the list
    char gameList[GAME_LIST_LENGTH][13] = {
        "PING",
        "FEEDME",
        "SPACE FLIGHT",
        "AMZNG MAZE",
        "GLIDER"
    };


	uint8_t selectedItem = 0;
	uint8_t repaint = 1;	// if true, the menu is repainted

	setScreen(BACKGROUND);
	char word[] = "SELECT GAME";
	writeString(word,5,3,RED);

	while(1)
	{		
		uint8_t yPos = 10;
		uint8_t start = (selectedItem / SCREEN_LENGTH) * SCREEN_LENGTH;
        uint8_t end = (start + SCREEN_LENGTH) > GAME_LIST_LENGTH-1 ? GAME_LIST_LENGTH-1 : (start + SCREEN_LENGTH - 1);
		uint8_t i;

		// paint new menu
        if(repaint)
        {
			repaint = 0;
			for(i = start; i <= end; i++)
			{
				if(i == selectedItem)
				{
                    writeString(gameList[i],5,yPos, SELECT_COLOR);
				}
				else
				{
                    writeString(gameList[i],5,yPos, MENU_COLOR);
				}
				yPos += 6; // next line
			}
		}

		// handle buttons
		if(isPushed(BUTTON_UP))
		{
			// don't let selected item be smaller than zero
			if(selectedItem >= 1)
			{
				selectedItem--;
				uint8_t newStart = (selectedItem / SCREEN_LENGTH) * SCREEN_LENGTH;
				// if another page of the menu has to be displayed: repaint
				if(start != newStart)
				{
					repaint = 1;
				}
				else // only repaint selected item and new selected item
				{
                    writeString(gameList[selectedItem+1],5,10+6*((selectedItem+1)%5), MENU_COLOR); // previously selected
                    writeString(gameList[selectedItem],5,10+6*(selectedItem%5), SELECT_COLOR); // now selected item
				}
			}
		}
		if(isPushed(BUTTON_DOWN))
		{
			// don't let selected item be greater than max. possible item
			if(selectedItem < GAME_LIST_LENGTH-1)
			{
				selectedItem++;
				// if another page of the menu has to be displayed: repaint
                if(selectedItem > end)
				{
					repaint = 1;
				}
				else // only repaint selected item and new selected item
				{
                    writeString(gameList[selectedItem],5,10+6*((selectedItem)%5), SELECT_COLOR); // now selected item
                    writeString(gameList[selectedItem-1],5,10+6*((selectedItem-1)%5), MENU_COLOR); // previously selected
				}
			}
		}
		if(isPushed(BUTTON_ZERO)||isPushed(BUTTON_ONE)||isPushed(BUTTON_TWO))
		{
			return selectedItem;
		}

		// delete old menu if new page of menu opened
		if(repaint)
		{
			yPos = 10;
			for(i = start; i <= end; i++)
			{
                writeString(gameList[i],5,yPos, BACKGROUND);
				yPos += 6; // next line
			}
		}
        wait(0);// only for the simulator
	}
}


void showHighScoreMenu(uint16_t score)
{
#define NUM_LETTERS     4
#define NUM_NAMES       4

    setScreen(BLACK);

    // load highscores from EEPROM here
    // load 4 names, each with 4 chars
    char names[NUM_NAMES][NUM_LETTERS+1] = {"SUSI", "MONA", "VIS", "ABC"};
    // load 4 scores
    uint16_t scores[NUM_NAMES];


    // dummy data
    scores[0]= 10;
    scores[1]= 5;
    scores[2]= 3;
    scores[3]= 2;


    // find out, whether player has got a highscore
    uint8_t place = NUM_NAMES;
    uint8_t cnt;
    for(cnt = 0; cnt < NUM_NAMES; cnt++)
    {
        if(score >= scores[cnt])
        {
            place = cnt; // place has the index number of the highscore placement
            break;
        }
    }

    // convert score int to char for display
    char scoreString[6];
    convertToString(score, scoreString, 6);

    if(place >= NUM_NAMES)
    {
        // no highscore
        writeString("NO HIGHSCORE", 1, 5, RED);
        writeString("TRY AGAIN!", 7, 15, RED);

        writeString("YOUR SCORE:", 5, 25, PALE_VIOLET);
        writeString(scoreString, 5, 32, PALE_VIOLET);

        wait(500);
        // wait for button press
        while(!isPushed(ANY_BUTTON))
        {
            wait(0);// wait is only for the simulator
        }
    }
    else
    {
        // highscore
        writeString("WELL DONE!", 8, 5, FRESH);

        writeString("YOUR SCORE:", 5, 20, PALE_VIOLET);
        writeString(scoreString, 5, 27, PALE_VIOLET);

        wait(500);
        // wait for button press
        while(!isPushed(ANY_BUTTON))
        {
            wait(0);// wait is only for the simulator
        }
        setScreen(BLACK);
        writeString("ENTER NAME_       FOR_HIGHSCORE", 5, 5, FRESH);

        uint8_t currentLetter = 0;
        uint8_t repaint = 1;
        uint8_t letterPositions[NUM_LETTERS] = {15, 22, 29, 36};
        // store new name
        char newName[NUM_LETTERS+1] = "    ";
        // temp string for display
        char temp[2]; temp[1] = '\0';

        wait(200);
        while(1)
        {
            if(repaint)
            {
                // paint placeholders for all letters
                for(cnt = 0; cnt < NUM_LETTERS; cnt++)
                {
                    temp[0] = newName[cnt];
                    writeString(temp,letterPositions[cnt]-2,28,PALE_VIOLET);
                    setPixel(letterPositions[cnt],34,PALE_VIOLET);
                }
                setPixel(letterPositions[currentLetter],34,FRESH);

                repaint = 0;
            }

            if(isPushed(ANY_BUTTON))
            {
                // change between letter positions
                if(isPushed(BUTTON_LEFT))
                {
                    if(currentLetter >= 1) currentLetter--;
                }
                else if(isPushed(BUTTON_RIGHT))
                {
                    if(currentLetter < 3) currentLetter++;
                }

                // change current letter
                if(isPushed(BUTTON_UP))
                {
                    temp[0] = newName[currentLetter];
                    writeString(temp,letterPositions[currentLetter]-2,28,BLACK);

                    newName[currentLetter]--;
                    if(newName[currentLetter] == '@') newName[currentLetter] = ' ';
                    if(newName[currentLetter] == '/') newName[currentLetter] = 'Z';
                    if(newName[currentLetter] < ' ') newName[currentLetter] = '9';
                }
                else if(isPushed(BUTTON_DOWN))
                {

                    temp[0] = newName[currentLetter];
                    writeString(temp,letterPositions[currentLetter]-2,28,BLACK);

                    newName[currentLetter]++;
                    if(newName[currentLetter] == '!') newName[currentLetter] = 'A';
                    if(newName[currentLetter] > 'Z') newName[currentLetter] = '0';
                    if(newName[currentLetter] == ':') newName[currentLetter] = ' ';
                }

                if(isPushed(BUTTON_ONE) || isPushed(BUTTON_TWO))
                {
                    break;
                }
                repaint = 1;
            }
            wait(0);// wait is only for the simulator
        }

        // put new name into highscore list
        uint8_t cnt2;
        for(cnt = NUM_NAMES-1; cnt >= place && cnt != 255; cnt--)
        {
            if(cnt == place)
            {
                for(cnt2 = 0; cnt2 < NUM_LETTERS; cnt2++)
                {
                    names[cnt][cnt2] = newName[cnt2];
                }
                scores[cnt] = score;
            }
            else
            {
                for(cnt2 = 0; cnt2 < NUM_LETTERS; cnt2++)
                {
                    names[cnt][cnt2] = names[cnt-1][cnt2];
                }
                scores[cnt]= scores[cnt-1];
            }
        }
    }

    // display resulting highscore list
    setScreen(BLACK);
    writeString("HIGHSCORE", 8, 3, FRESH);
    uint8_t height = 12;
    uint8_t color;
    for(cnt = 0; cnt < NUM_NAMES; cnt++)
    {
        color = PALE_VIOLET;
        if(cnt == place) color = FRESH;
        writeString(names[cnt],3,height,color);
        convertToString(scores[cnt], scoreString, 6);
        writeString(scoreString,29,height,color);
        height += 7;
    }

    // save data to EEPROM here

    wait(1000);
    // before return wait for button press
    while(!isPushed(ANY_BUTTON))
    {
        wait(0);// wait is only for the simulator
    }
}


void loadGame(uint8_t gameNum)
{
	// load selected item
	// New Game Step 4: add the main function of your game to switch statement
	switch(gameNum){
    case 0: ping_startGame();break;
    case 1: feedme_startGame();break;
    case 2: spaceFlight_startGame();break;
    case 3: amazingMaze_startGame();break;
    case 4: glider_startGame();break;
    default: break;
	}
}


void showGameTitle(char* title, uint8_t stringColor, uint8_t backgroundColor, uint8_t x, uint8_t y)
{
	setScreen(backgroundColor);
	writeString(title,x,y,stringColor);
    while(!isPushed(ANY_BUTTON))
    {
        wait(0);// only for the simulator
    }
    wait(200);
}


void checkForReset()
{
	if(mResetCounter == 0)
	{
		mResetControlTime = getTimestamp();
		mResetCounter++;
	}
	else
	{
		if(getTimestamp() - mResetControlTime <= 100)
		{
			mResetControlTime = getTimestamp();
			mResetCounter++;
		}
		else
		{
			mResetCounter = 0;
		}
	}

	// black out the screen line by line from above
	if(mResetCounter >= 25)
	{
		uint8_t i = 0;
        mResetCounter = 0;
        mResetControlTime = 0;
		for(i = 0; i < LINES; i++)
		{
            setCompleteHLine(i, BLACK);
			wait(40);
		}
		wait(250);
		reset();
	}
}


void showOptionsMenu(char* optionsText, uint8_t xPos, uint8_t yPos,
                     uint8_t* firstOption, uint8_t minValFirst, uint8_t maxValFirst, uint8_t stdValFirst,
                     uint8_t* secondOption, uint8_t minValSecond, uint8_t maxValSecond, uint8_t stdValSecond)

{
	if(xPos < 3) xPos = 3; // leave room for the selection marker

    setScreen(BLACK);
    writeString("OPTIONS",11,3,MINT);
    writeString(optionsText,xPos,yPos,SEA_BLUE);

    *firstOption = stdValFirst;
	*secondOption = stdValSecond;

    uint8_t repaint = 1;
    uint8_t selected = 0;
    char first[2];
    char second[2];

    first[0] = *firstOption+48; // convert number to corresponding ascii code
    first[1] = 0; // zero string terminator
    second[0] = *secondOption+48; // convert number to corresponding ascii code
    second[1] = 0; // zero string terminator

    while(1)
    {
        if(repaint)
        {
            repaint = 0;
            first[0] = *firstOption+48;
            writeString(first,xPos+26,yPos,MINT);
            second[0] = *secondOption+48;
            writeString(second,xPos+26,yPos+12,MINT);

            if(selected % 2 == 0)
            {
                setPixel(xPos-3,yPos+2,RED);setPixel(xPos-3,yPos+2+12,BLACK);
            }
            else
            {
                setPixel(xPos-3,yPos+2+12,RED);setPixel(xPos-3,yPos+2,BLACK);
            }
        }

        if(isPushed(ANY_BUTTON))
        {
            repaint = 1;
            if(isPushed(BUTTON_UP))
            {
                selected--;wait(160);
            }
            if(isPushed(BUTTON_DOWN))
            {
                selected++;wait(160);
            }
            if(isPushed(BUTTON_LEFT))
            {
                wait(160);
                if(selected % 2 == 0)
                {
                    if(*firstOption > minValFirst)
                    {
                        writeString(first,xPos+26,yPos,BLACK);
                        (*firstOption)--;
                    }
                }
                else
                {
                    if(*secondOption > minValSecond)
                    {
                        writeString(second,xPos+26,yPos+12,BLACK);
                        (*secondOption)--;
                    }
                }
            }
            if(isPushed(BUTTON_RIGHT))
            {
                wait(160);
                if(selected % 2 == 0)
                {
                    if(*firstOption < maxValFirst)
                    {
                        writeString(first,xPos+26,yPos,BLACK);
                        (*firstOption)++;
                    }
                }
                else
                {
                    if(*secondOption < maxValSecond)
                    {
                        writeString(second,xPos+26,yPos+12,BLACK);
                        (*secondOption)++;
                    }
                }
            }
            if(isPushed(BUTTON_ZERO) || isPushed(BUTTON_ONE) || isPushed(BUTTON_TWO))
            {
                break;
            }
        }
        wait(0);// only for the simulator
    }

}

