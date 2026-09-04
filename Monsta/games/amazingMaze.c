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
#include "amazingMaze.h"
#include <string.h> // only for the simulator: provides memset

#define MAZE_NUM_SEGMENTS 4
#define AMAZING_MAZE_BACKGROUND BLACK
uint8_t amazingMazeForeground; // defined via user input, standard value is GREEN

#define AMAZING_MAZE_MARKER_A   VIOLET
#define AMAZING_MAZE_MARKER_B   DARK_RED

// boundaries to paint the closest segment of the maze
#define LEFT_X 7
#define TOP_Y 0
#define BOTTOM_Y 41
#define RIGHT_X 48 //(55-LEFT_X)

struct gameStateStruct
{
    uint8_t paused: 1;
    uint8_t gameOver: 1;
    uint8_t nextLevel: 1;
};
typedef struct gameStateStruct gameStateStruct;

// path in front of the player
static uint8_t frontPath[MAZE_NUM_SEGMENTS+1];

// represents the state of the player in the maze
struct mazePlayer
{
    uint8_t xPos: 4;
    uint8_t yPos: 4;
    uint8_t lookAt: 2; // 00 = up, 01 = left, 10 = down, 11 = right
    uint8_t doRepaint: 1;
    uint8_t hasKey: 1;
    uint8_t hasHammer: 1;
    uint8_t currentLevel: 3; // the maze level the player is in
};
typedef struct mazePlayer mazePlayer;

// represents the map of the maze
struct mazeMap
{
    // start position of player
    uint8_t startX: 4;
    uint8_t startY: 4;
    // exit position
    uint8_t exitX: 4;
    uint8_t exitY: 4;
    // cracked walls
    uint8_t crackedWall_1_X; // positions where the cracked wall is seen from,
    uint8_t crackedWall_2_X; // not the cracked wall itself!
    uint8_t crackedWall_1_Y: 4;
    uint8_t crackedWall_2_Y: 4;
    uint8_t crackedWall_1_Dir: 2; // directino in which the craced wall is seen
    uint8_t crackedWall_2_Dir: 2;
    uint8_t crackedWall_1_Hit: 2;
    uint8_t crackedWall_2_Hit: 2;
    // marker set by player
    uint8_t markerAx; // whole byte: enables using 255 as invalid value
    uint8_t markerBx;
    uint8_t markerAy: 4;
    uint8_t markerBy: 4;
    // key
    uint8_t keyX; // whole byte: enables using 255 as invalid value
    uint8_t keyY: 4;
    // hammer
    uint8_t hammerX; // whole byte: enables using 255 as invalid value
    uint8_t hammerY: 4;
    // exit orientation (exit is only shown if player has this orientation)
    uint8_t exitDir: 2;
    // indicates which marker was set last
    uint8_t lastMarkerSetIsA: 1;

    // map is an array of bytes: each set bit is a wall, each unset bit is a path
    // example: -----------------> x
    //          | 10001001 00110100 (first and second bytes of array)
    //          | 11000011 01100010 (third and fourth bytes of array)
    //          | ...
    //          | .
    //          | .
    //        y V
    uint8_t* mapData;
};
typedef struct mazeMap mazeMap;


static gameStateStruct gameState;
static mazeMap map;
static mazePlayer player;

// tests whether the player is looking along the horzontal map axis
#define PLAYER_IS_HORIZONTAL() ((uint8_t) ((player.lookAt) % 2))
// tests whether the player is looking along the positive or negative axis direction
#define PLAYER_IS_POSITIVE() ((uint8_t) ((player.lookAt) >=2))
// find increment for map readout to iterate over looking axis
#define PLAYER_GET_INCREMENT() ((uint8_t) ((PLAYER_IS_POSITIVE()) ? 1 : 255)) // 255 is -1


static uint8_t getMapValue(uint8_t x, uint8_t y)
{
    if(x >= 16) return 1; // catch errors (too high and too low values)
    if(y >= 16) return 1; // remember: low values (e.g. -1) is 255

    // read out bit at position (x,y) in the map
    if(x < 8)
    {
        return map.mapData[y*2] & (1 << (7-x));
    }
    else
    {
        return map.mapData[y*2+1] & (1 << (15-x));
    }
}

static void removeWall()
{
    // remove cracked wall from status variables
    if(player.xPos == map.crackedWall_1_X && player.yPos == map.crackedWall_1_Y)
    {
        map.crackedWall_1_X = 255;
    }
    else // --> wall 2, implementation relies on the fact that this function is only called when a real cracked wall is to be removed!
    {
        map.crackedWall_2_X = 255;
    }

    uint8_t tempX, tempY;
    uint8_t increment = PLAYER_GET_INCREMENT();

    // go one step along the looking direction
    if(PLAYER_IS_HORIZONTAL())
    {
        tempX = player.xPos + increment;
        tempY = player.yPos;
    }
    else
    {
        tempX = player.xPos;
        tempY = player.yPos + increment;
    }

    // remove cracked wall from map data
    if(tempX < 8)
    {
        map.mapData[tempY*2] ^= (1 << (7-tempX));
    }
    else
    {
        map.mapData[tempY*2+1] ^= (1 << (15-tempX));
    }
}

// maps for the maze: every map row consists of two consecutive bytes --> map size is 16 x 16 bits
static uint8_t map01[32] = {33, 4, 109, 82, 7, 8, 84, 173, 66, 144, 22, 214, 178, 132, 8, 178,
                            247, 77, 42, 32, 138, 170, 81, 2, 6, 172, 82, 33, 137, 85, 36, 132};
static uint8_t map02[32] = {41, 32, 100, 138, 41, 182, 107, 0, 0, 213, 218, 136, 71, 34, 17, 138,
                            69, 45, 150, 148, 121, 33, 35, 90, 136, 136, 43, 34, 66, 150, 24, 162};

static void initGame(uint8_t state)
{
    //timeInc = 4; // set higher delay than normal after a button was pushed - not available in the simulator

    // set game flags
    gameState.nextLevel = 0;

    // game is starting
    if(state == 0)
    {
        gameState.gameOver = 0;
        gameState.paused = 0;
    }
    else // game is continuing
    {
        // last level completed
        if(player.currentLevel == 5) // to add more levels: replace 5 by new number
        {
            gameState.gameOver = 1;
            return;
        }
        else
        {
            player.currentLevel++;
        }
    }

    char text[8];
    text[0] = 'L';
    text[1] = 'E';
    text[2] = 'V';
    text[3] = 'E';
    text[4] = 'L';
    text[5] = ' ';
    convertToString(player.currentLevel+1,&text[6],2); // sets text[6] and text[7]
    // this function is used to announce the level
    showGameTitle(text, MINT, BLACK, 14, 15);

    // delete screen
    setScreen(AMAZING_MAZE_BACKGROUND);

    // set initial map attributes common for all levels
    map.markerAx = 255; // set invalid marker position initially
    map.markerAy = 0;
    map.markerBx = 255; // set invalid marker position initially
    map.markerBy = 0;
    map.lastMarkerSetIsA = 0;

    // set initial map attributes for current level
    switch(player.currentLevel)
    {
    case 0:
        map.startX = 6;
        map.startY = 1;
        map.exitX = 6;
        map.exitY = 3;
        map.exitDir = 1;
        map.keyX = 6;
        map.keyY = 1;
        map.hammerX = 255; // not relevant for this level
        map.crackedWall_1_X = 255; // not relevant for this level
        map.crackedWall_2_X = 255; // not relevant for this level
        map.mapData = map01;
        break;
    case 1:
        map.startX = 8;
        map.startY = 2;
        map.exitX = 14;
        map.exitY = 6;
        map.exitDir = 1;
        map.keyX = 14;
        map.keyY = 0;
        map.hammerX = 255; // not relevant for this level
        map.crackedWall_1_X = 255; // not relevant for this level
        map.crackedWall_2_X = 255; // not relevant for this level
        map.mapData = map01;
        break;
    case 2:
        map.startX = 11;
        map.startY = 11;
        map.exitX = 7;
        map.exitY = 12;
        map.exitDir = 0;
        map.keyX = 10;
        map.keyY = 14;
        map.hammerX = 255; // not relevant for this level
        map.crackedWall_1_X = 255; // not relevant for this level
        map.crackedWall_2_X = 255; // not relevant for this level
        map.mapData = map01;
        break;
    case 3:
        map.startX = 6;
        map.startY = 11;
        map.exitX = 1;
        map.exitY = 0;
        map.exitDir = 3;
        map.keyX = 4;
        map.keyY = 8;
        map.hammerX = 4;
        map.hammerY = 15;
        map.crackedWall_1_X = 3;
        map.crackedWall_1_Y = 9;
        map.crackedWall_1_Dir = 0;
        map.crackedWall_1_Hit = 0;
        map.crackedWall_2_X = 255; // not relevant for this level
        map.mapData = map01;
        break;
    case 4:
        map.startX = 4;
        map.startY = 4;
        map.exitX = 7;
        map.exitY = 14;
        map.exitDir = 0;
        map.keyX = 6;
        map.keyY = 10;
        map.hammerX = 7;
        map.hammerY = 1;
        map.crackedWall_1_X = 1;
        map.crackedWall_1_Y = 9;
        map.crackedWall_1_Dir = 1;
        map.crackedWall_1_Hit = 0;
        map.crackedWall_2_X = 255; // not relevant for this level
        map.mapData = map02;
        break;
    case 5:
        map.startX = 12;
        map.startY = 9;
        map.exitX = 13;
        map.exitY = 1;
        map.exitDir = 2;
        map.keyX = 12;
        map.keyY = 4;
        map.hammerX = 10;
        map.hammerY = 14;
        map.crackedWall_1_X = 8;
        map.crackedWall_1_Y = 13;
        map.crackedWall_1_Dir = 1;
        map.crackedWall_1_Hit = 0;
        map.crackedWall_2_X = 6;
        map.crackedWall_2_Y = 10;
        map.crackedWall_2_Dir = 3;
        map.crackedWall_2_Hit = 0;
        map.mapData = map02;
        break;
    }


    // create player
    player.xPos = map.startX;
    player.yPos = map.startY;
    player.lookAt = 0;
    player.doRepaint = 1;
    player.hasKey = 0;
    player.hasHammer = 0;
}


// find out how far (how many segments) the player can see (0 to MAZE_NUM_SEGMENTS)
static uint8_t getSightDist()
{
    // save the properties of the path in front of the player
    memset(&frontPath, 0, MAZE_NUM_SEGMENTS+1); // init frontPath with 0

    // find increment for map readout to iterate over positions in the map
    uint8_t increment = PLAYER_GET_INCREMENT();

    uint8_t sightDist;
    for(sightDist = 0; sightDist < MAZE_NUM_SEGMENTS+1; sightDist++)
    {
        // check area in front of the player
        uint8_t tempX, tempY;

        // iterate along the looking direction
        if(PLAYER_IS_HORIZONTAL())
        {
            tempX = player.xPos + (sightDist*increment);
            tempY = player.yPos;
        }
        else
        {
            tempX = player.xPos;
            tempY = player.yPos + (sightDist*increment);
        }

        // check area in front of the player
        if(getMapValue(tempX, tempY))
        {
            break; // break on first set bit along the looking direction: wall, no need to look further
        }
        // check if segment is "marker a" (marker set by player for orientation)
        if(map.markerAx == tempX && map.markerAy == tempY)
        {
            frontPath[sightDist] = 'a';
        }
        // check if segment is "marker b" (marker set by player for orientation)
        if(map.markerBx == tempX && map.markerBy == tempY)
        {
            frontPath[sightDist] = 'b';
        }
        // check if segment is key
        if(map.keyX == tempX && map.keyY == tempY)
        {
            frontPath[sightDist] = 'k';
        }
        // check if segment is hammer
        if(map.hammerX == tempX && map.hammerY == tempY)
        {
            frontPath[sightDist] = 'h';
        }
        // check if segment is exit
        if(map.exitX == tempX && map.exitY == tempY)
        {
            frontPath[sightDist] = 'e';
        }
        // check if segment has cracked wall
        if(map.crackedWall_1_X == tempX && map.crackedWall_1_Y == tempY)
        {
            frontPath[sightDist] = 'c';
        }
        if(map.crackedWall_2_X == tempX && map.crackedWall_2_Y == tempY)
        {
            frontPath[sightDist] = 'd';
        }
    }

    // if the for loop was left by the break statement, there is a wall within the player's max sight
    // (sightDist is in [0|4])
    // sightDist == 0 means the player is standing inside a wall (should not occur)
    // sightDist == 1 means the player is looking directly at a wall; when painting, this case is accessed with
    // the index 0 in the array of offsets, therefore sightDist is decremented to access the right array element
    sightDist--;
    // sightDist is now theoretically in [-1|3], practically in [0|3]
    // if the loop is not left with break: the wall is further away than the player can look
    // (sightDist == 4 after decrement)

    return sightDist;
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
        if(isPushed(BUTTON_UP)) // go ahead
        {
            if(getSightDist() > 0)
            {
                if(PLAYER_IS_HORIZONTAL())
                {
                    player.xPos += PLAYER_GET_INCREMENT();
                    player.doRepaint = 1; // state changed, request repaint
                }
                else
                {
                    player.yPos += PLAYER_GET_INCREMENT();
                    player.doRepaint = 1; // state changed, request repaint
                }
            }
        }
        if(isPushed(BUTTON_DOWN)) // go back
        {
            // simulate look backwards
            player.lookAt += 2;
            if(getSightDist() > 0)
            {
                if(PLAYER_IS_HORIZONTAL())
                {
                    player.xPos += PLAYER_GET_INCREMENT();
                    player.doRepaint = 1; // state changed, request repaint
                }
                else
                {
                    player.yPos += PLAYER_GET_INCREMENT();
                    player.doRepaint = 1; // state changed, request repaint
                }
            }
            // undo look backwards
            player.lookAt += 2;
        }
        if(isPushed(BUTTON_LEFT)) // turn left
        {
            player.lookAt++;
            player.doRepaint = 1; // state changed, request repaint
        }
        if(isPushed(BUTTON_RIGHT)) // turn right
        {
            player.lookAt--;
            player.doRepaint = 1; // state changed, request repaint
        }

        // use item
        if(isPushed(BUTTON_ONE))
        {
            player.doRepaint = 1; // state changed, request repaint
            // use key
            if(player.xPos == map.exitX && player.yPos == map.exitY)
            {
                // open door
                if(player.hasKey)
                {
					playSong(SHORT_HIGH_BEEP);
                    player.hasKey = 0;
                    gameState.nextLevel = 1; // go to next level flag
                }
            }

            // use hammer
            if(player.xPos == map.crackedWall_1_X && player.yPos == map.crackedWall_1_Y &&
               player.lookAt == map.crackedWall_1_Dir)
            {
                if(player.hasHammer)
                {
					playSong(KLICK1);
                    map.crackedWall_1_Hit++;
                    if(map.crackedWall_1_Hit == 0) // 2 bits overflow: wall collapses
                    {
						playSong(EXPLODE);
                        removeWall();
                    }
                }
            }
            if(player.xPos == map.crackedWall_2_X && player.yPos == map.crackedWall_2_Y &&
               player.lookAt == map.crackedWall_2_Dir)
            {
                if(player.hasHammer)
                {
					playSong(KLICK1);
                    map.crackedWall_2_Hit++;
                    if(map.crackedWall_2_Hit == 0) // 2 bits overflow: wall collapses
                    {
						playSong(EXPLODE);
                        removeWall();
                    }
                }
            }
        }

        // set markers
        if(isPushed(BUTTON_TWO))
        {
            player.doRepaint = 1; // state changed, request repaint
			playSong(KLICK2);
            if(!map.lastMarkerSetIsA)
            {
                map.markerAx = player.xPos;
                map.markerAy = player.yPos;
                map.lastMarkerSetIsA = 1;
            }
            else
            {
                map.markerBx = player.xPos;
                map.markerBy = player.yPos;
                map.lastMarkerSetIsA = 0;
            }
        }
    }
}

static void paintKey(uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t keyCenterX = x;
    uint8_t keyCenterY = y;
    paintRectangle(keyCenterX-1, keyCenterY-1, keyCenterX+1, keyCenterY+1, color);
    setVLine(keyCenterX, keyCenterY+2, keyCenterY+5, color);
    setPixel(keyCenterX-1, keyCenterY+3, color);
    setPixel(keyCenterX-1, keyCenterY+5, color);
}

static void repaint()
{   
    if(player.doRepaint)
    {
        player.doRepaint = 0;
        setScreen(AMAZING_MAZE_BACKGROUND);

        { // --- paint compass ---
            uint8_t compassX = 3;
            uint8_t compassY = 7;
            // paint all directions, different color for looking direction
            setPixel(compassX, compassY-1, player.lookAt == 0 ? ORANGE : WHITE);
            setPixel(compassX-1, compassY, player.lookAt == 1 ? ORANGE : WHITE);
            setPixel(compassX, compassY+1, player.lookAt == 2 ? ORANGE : WHITE);
            setPixel(compassX+1, compassY, player.lookAt == 3 ? ORANGE : WHITE);
        }

        // --- paint key ---
        if(player.hasKey)
        {
            paintKey(3, 12, YELLOW); // paint key on the left side (menu)
        }
        // --- paint hammer ---
        if(player.hasHammer) // paint hammer on the left side (menu)
        {
            paintRectangle(2, 20, 5, 21, GRAY);
            paintRectangle(3, 22, 4, 27, DARK_RED);
        }

        // init containers for walls: these will be overwritten by actual values in the map and displayed afterwards
        uint8_t wallsLeft[MAZE_NUM_SEGMENTS] = {1, 1, 1, 1};
        uint8_t wallsRight[MAZE_NUM_SEGMENTS] = {1, 1, 1, 1};

        // find out how far (how many segments) the player can see (0 to MAZE_NUM_SEGMENTS)
        uint8_t sightDist = getSightDist();

        // find increment for map readout to iterate over positions in the map
        uint8_t increment = PLAYER_GET_INCREMENT();

        // read surroundings of player from map
        uint8_t counter;
        for(counter = 0; counter < MAZE_NUM_SEGMENTS; counter++)
        {
            // iterate along the looking direction
            if(PLAYER_IS_HORIZONTAL())
            {
                uint8_t tempXPos = player.xPos + (counter*increment);
                // read and save values on the left and right side of the player
                if(PLAYER_IS_POSITIVE())
                {
                    wallsRight[counter] = getMapValue(tempXPos, player.yPos + 1);
                    wallsLeft[counter] = getMapValue(tempXPos, player.yPos - 1);
                }
                else
                {
                    wallsRight[counter] = getMapValue(tempXPos, player.yPos - 1);
                    wallsLeft[counter] = getMapValue(tempXPos, player.yPos + 1);
                }
            }
            else
            {
                uint8_t tempYPos = player.yPos + (counter*increment);
                // read and save values on the left and right side of the player
                if(PLAYER_IS_POSITIVE())
                {
                    wallsRight[counter] = getMapValue(player.xPos - 1, tempYPos);
                    wallsLeft[counter] = getMapValue(player.xPos + 1, tempYPos);
                }
                else
                {
                    wallsRight[counter] = getMapValue(player.xPos + 1, tempYPos);
                    wallsLeft[counter] = getMapValue(player.xPos - 1, tempYPos);
                }
            }
        }

        // offsets for the different segments
        static uint8_t offsets[MAZE_NUM_SEGMENTS] = {3, 10, 15, 18};

        // only paint as far as the player can see
        uint8_t limit = sightDist >= MAZE_NUM_SEGMENTS ? MAZE_NUM_SEGMENTS : sightDist+1;
        // paint surroundings of player
        for(counter = 0; counter < limit; counter++)
        {
            // left side
            if(wallsLeft[counter]) // wall
            {
                // draw diagonal line in segment
                if(counter == 0)
                {
                    // top left
                    drawLine(LEFT_X, TOP_Y, LEFT_X + offsets[counter], TOP_Y + offsets[counter], amazingMazeForeground);
                    // bottom left
                    drawLine(LEFT_X,BOTTOM_Y, LEFT_X+offsets[counter],BOTTOM_Y-offsets[counter],amazingMazeForeground);
                }
                else
                {
                    // top left
                    drawLine(LEFT_X + offsets[counter-1] + 1, TOP_Y + offsets[counter-1] + 1,
                             LEFT_X + offsets[counter], TOP_Y + offsets[counter],amazingMazeForeground);
                    // bottom left
                    drawLine(LEFT_X + offsets[counter-1] + 1, BOTTOM_Y - offsets[counter-1] - 1,
                             LEFT_X+offsets[counter],BOTTOM_Y-offsets[counter],amazingMazeForeground);
                }
            }
            else // path
            {
                // vertical lines
                if(counter != 0)
                {
                    setVLine(LEFT_X+offsets[counter-1], TOP_Y+offsets[counter-1], BOTTOM_Y-offsets[counter-1], amazingMazeForeground);
                }
                setVLine(LEFT_X+offsets[counter], TOP_Y+offsets[counter], BOTTOM_Y-offsets[counter], amazingMazeForeground);

                // horizontal lines
                if(counter != 0)
                {
                    setHLine(LEFT_X+offsets[counter-1]+1, LEFT_X+offsets[counter], TOP_Y+offsets[counter],amazingMazeForeground);
                    setHLine(LEFT_X+offsets[counter-1]+1, LEFT_X+offsets[counter], BOTTOM_Y-offsets[counter],amazingMazeForeground);
                }
                else
                {
                    setHLine(0, LEFT_X+offsets[counter], TOP_Y+offsets[counter],amazingMazeForeground);
                    setHLine(0, LEFT_X+offsets[counter], BOTTOM_Y-offsets[counter],amazingMazeForeground);
                }
            }


            // right side
            if(wallsRight[counter]) // wall
            {
                // draw diagonal line in segment
                if(counter == 0)
                {
                    // top right
                    drawLine(RIGHT_X,TOP_Y, RIGHT_X-offsets[counter], TOP_Y+offsets[counter], amazingMazeForeground);
                    // bottom right
                    drawLine(RIGHT_X,BOTTOM_Y, RIGHT_X-offsets[counter],BOTTOM_Y-offsets[counter], amazingMazeForeground);
                }
                else
                {
                    // top right
                    drawLine(RIGHT_X-offsets[counter-1] - 1, TOP_Y+offsets[counter-1] + 1,
                             RIGHT_X-offsets[counter], TOP_Y+offsets[counter], amazingMazeForeground);
                    // bottom right
                    drawLine(RIGHT_X-offsets[counter-1]-1, BOTTOM_Y-offsets[counter-1]-1,
                             RIGHT_X-offsets[counter], BOTTOM_Y-offsets[counter], amazingMazeForeground);
                }
            }
            else // path
            {
                // vertical lines
                if(counter != 0)
                {
                    setVLine(RIGHT_X-offsets[counter-1], TOP_Y+offsets[counter-1], BOTTOM_Y-offsets[counter-1],amazingMazeForeground);
                }
                setVLine(RIGHT_X-offsets[counter], TOP_Y+offsets[counter], BOTTOM_Y-offsets[counter],amazingMazeForeground);

                // horizontal lines
                if(counter != 0)
                {
                    setHLine(RIGHT_X-offsets[counter], RIGHT_X-offsets[counter-1]-1, TOP_Y+offsets[counter], amazingMazeForeground);
                    setHLine(RIGHT_X-offsets[counter], RIGHT_X-offsets[counter-1]-1, BOTTOM_Y-offsets[counter], amazingMazeForeground);
                }
                else
                {
                    setHLine(RIGHT_X-offsets[counter], COLUMNS-1, TOP_Y+offsets[counter], amazingMazeForeground);
                    setHLine(RIGHT_X-offsets[counter], COLUMNS-1, BOTTOM_Y-offsets[counter], amazingMazeForeground);
                }
            }
        }

        // paint end
        if(sightDist < MAZE_NUM_SEGMENTS)
        {
            // end is within MAZE_NUM_SEGMENTS segments from player
            paintRectangle(LEFT_X+offsets[sightDist], TOP_Y+offsets[sightDist],
                           RIGHT_X-offsets[sightDist], BOTTOM_Y-offsets[sightDist], amazingMazeForeground);
        }
        else
        {
            // end is further away than MAZE_NUM_SEGMENTS segments: draw a rectangle in the center
            paintFilledRectangle(((LEFT_X+RIGHT_X-1)/2)-1, ((TOP_Y+BOTTOM_Y-1)/2)-1,
                                 ((LEFT_X+RIGHT_X+1)/2)+1, ((TOP_Y+BOTTOM_Y+1)/2)+1, amazingMazeForeground);
        }

        // paint items on the floor
        // modified sightDist
        uint8_t sightDistMod = sightDist < MAZE_NUM_SEGMENTS ? sightDist : MAZE_NUM_SEGMENTS-1;
        // paint elements on the path of the player
        for(counter = 0; counter <= sightDistMod; counter++) // counter <= sightDist is needed for correct array access
        {
            // frontPath[0] is the field the player is standing on

            // paint exit if player is looking towards it
            if(frontPath[counter] == 'e' && player.lookAt == map.exitDir)
            {
                paintFilledRectangle(LEFT_X+offsets[sightDistMod]+1, TOP_Y+offsets[sightDistMod]+1,
                                     RIGHT_X-offsets[sightDistMod]-1, BOTTOM_Y-offsets[sightDistMod]-1, BLUE);
                if(counter == 0)
                {
                    writeString("EXIT", LEFT_X+offsets[counter]+10, TOP_Y+offsets[counter]+2, WHITE);
                    if(gameState.nextLevel) // if game is over, the door is unlocked
                    {
                        paintKey(27, 20, YELLOW);
                        wait(1500); // pause to see inserted key in exit of current level
                    }
                    else // the door is still locked
                    {
                        paintKey(27, 20, BLACK);
                    }
                }
            }

            // paint cracked wall if player is looking towards it
            if(frontPath[counter] == 'c' && player.lookAt == map.crackedWall_1_Dir)
            {
                if(counter == 0)
                {
                    drawLine(22, 29, 32, 10, GRAY);
                    drawLine(19, 7, 34, 33, GRAY);
                    if(map.crackedWall_1_Hit > 0)
                    {
                        drawLine(19, 12, 29, 31, GRAY);
                    }
                    if(map.crackedWall_1_Hit > 1)
                    {
                        drawLine(13, 19, 41, 35, GRAY);
                    }
                    if(map.crackedWall_1_Hit > 2)
                    {
                        drawLine(13, 28, 39, 15, GRAY);
                    }
                }
                if(counter == 1)
                {
                    // TODO
                    drawLine(24, 26, 30, 13, GRAY);
                    drawLine(22, 13, 31, 28, GRAY);
                }
            }

            // paint cracked wall if player is looking towards it
            if(frontPath[counter] == 'd' && player.lookAt == map.crackedWall_2_Dir)
            {
                if(counter == 0)
                {
                    drawLine(22, 29, 32, 10, GRAY);
                    drawLine(19, 7, 34, 33, GRAY);
                    if(map.crackedWall_2_Hit > 0)
                    {
                        drawLine(19, 12, 29, 31, GRAY);
                    }
                    if(map.crackedWall_2_Hit > 1)
                    {
                        drawLine(13, 19, 41, 35, GRAY);
                    }
                    if(map.crackedWall_2_Hit > 2)
                    {
                        drawLine(13, 28, 39, 15, GRAY);
                    }
                }
            }

            // paint "marker a" or "marker b" (colored ground)
            if(frontPath[counter] == 'a' || frontPath[counter] == 'b')
            {
                uint8_t color = frontPath[counter] == 'a' ? AMAZING_MAZE_MARKER_A : AMAZING_MAZE_MARKER_B;
                // determine number of lines and starting point to paint
                uint8_t numLines;
                uint8_t startX;
                uint8_t startY;
                switch(counter)
                {
                case 0: numLines = 3; startX = 8;  startY = LINES-1; break;
                case 1: numLines = 7; startX = 11; startY = LINES-4; break;
                case 2: numLines = 5; startX = 18; startY = LINES-11; break;
                case 3: numLines = 3; startX = 23; startY = LINES-16; break;
                }
                uint8_t index;
                for(index = 0; index < numLines; index++)
                {
                    setHLine(startX+index, COLUMNS-1-(startX+index), startY-index, color);
                }
            }
            // paint key on the ground
            if(frontPath[counter] == 'k')
            {
                switch(counter)
                {
                case 0: player.hasKey = 1; map.keyX = 255; playSong(SHORT_VERY_HIGH_BEEP);
                    player.doRepaint = 1; break; // player takes key from the ground
                case 1: setHLine(27, 28, LINES-7, YELLOW); break;
                case 2: setHLine(27, 28, LINES-13, YELLOW); break;
                case 3: setPixel(27, LINES-17, YELLOW); break;
                }
            }
            // paint hammer on the ground
            if(frontPath[counter] == 'h')
            {
                switch(counter)
                {
                case 0: player.hasHammer = 1; map.hammerX = 255; playSong(SHORT_VERY_HIGH_BEEP);
                    player.doRepaint = 1; break; // player takes hammer from the ground
                case 1: paintFilledRectangle(27, LINES-8, 30, LINES-7, DARK_RED);
                    paintFilledRectangle(25, LINES-8, 26, LINES-7, GRAY);
                    break;
                case 2: setHLine(27, 28, LINES-13, DARK_RED);
                    setPixel(26, LINES-13, GRAY);
                    break;
                case 3: setHLine(27, 28, LINES-17, DARK_RED);
                    //setPixel(26, LINES-17, GRAY);
                    break;
                }
            }
        }

        wait(0); // wait is only for the simulator: omits crash
    }
}

static void computeNextStep()
{
}

static void handleGameOver()
{
    wait(2000);

    // handle game over here
    setScreen(BLUE);
    writeString("      JUHU!__  YOU FOUND_THE WAY OUT", 4, 8, YELLOW);

    wait(500);
    // before return wait for button press
    while(!isPushed(ANY_BUTTON))
    {
        wait(0);// wait is only for the simulator
    }

    //showHighScoreMenu(score);
}


void amazingMaze_startGame()
{
    showGameTitle("AMAZING_    MAZE", MINT, BLACK, 12, 14);
    showGameTitle("       THX TO__PETER DECKER__         FOR_INSPIRATION", MINT, BLACK, 4, 3);
	// set myOP1 to a value between 1 and 3, default 1 and set myOP2 to a value between 1 and 9, default 3
    {
        uint8_t level;
        showOptionsMenu("LEVEL__COLOR",14,15, &level, 1, 6, 1, &amazingMazeForeground, 1, 7, 4);
        // correct input: levels are counted internally from 0 to 7, user input is from 1 to 8
        player.currentLevel = level - 1;
        amazingMazeForeground *= 17; // correct for double buffer color code
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

