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
#include "spaceFlight.h"

#define SPACEFLIGHT_BACKGROUND   BLACK
#define UPDATE_LIMIT        16

#define SHIP_BODY_COLOR     BLUE //SEA_BLUE
#define SHIP_COCKPIT_COLOR  YELLOW //ORANGE
#define WEAPON_ITEM_COLOR   RED //LIGHT_RED
#define SHIELD_ITEM_COLOR   CYAN //DARK_CYAN

#define MIN_Y_POS 9 // minimal y coordinate (leaving space for scores etc. on top of the screen)

struct spaceFlightFlags
{
    uint8_t paused: 1;
    uint8_t gameOver: 1;
    uint8_t shipCollision: 2;
    uint8_t shield: 1;
};
typedef struct spaceFlightFlags spaceFlightFlags;

static spaceFlightFlags flags;

static uint8_t lifes; // remaining lifes

static uint8_t update = 0; // used to slow down calculation of the next step: outer loop
static uint8_t update2 = 0;// ... : inner loop

static uint8_t gameSpeed; // used to slow down or increase game speed (high values = slow)
static uint8_t objectDelay; // delays creation of objects for the given amount of game steps

static uint16_t score; // player's score
static uint16_t oldScore;

static uint8_t shipPosX;    // position of the ship
static uint8_t shipPosY;

//static Point2D shotPos; // position of the bullet

#define SPACEFLIGHT_NUM_SHOTS 5     // limit for maxShots
static uint8_t maxShots;// maximum number of shots that can be fired simultaneously
static Point2D allShots[SPACEFLIGHT_NUM_SHOTS]; // position of the bullets

#define SPACEFLIGHT_NUM_OBJECTS 6

struct Object
{
    Point2D position;
    uint8_t color;
    uint8_t type: 4;
    uint8_t moveDown: 1;
    uint8_t moveUp: 1;
};
typedef struct Object Object;

static Object allObjects[SPACEFLIGHT_NUM_OBJECTS];

static void paintShip(uint8_t color);
static void paintNumShots();

static void initGame(uint8_t reset)
{
    setScreen(SPACEFLIGHT_BACKGROUND);

    if(!reset) // init
    {
        score = 0;
        oldScore = 1; // init with different value than score
        lifes = 2;

        maxShots = 1;

        flags.gameOver = 0;
        flags.paused = 0;
    }
    else // reset
    {
        if(score == oldScore)
        {
            oldScore +=1;
        }
    }

    flags.shipCollision = 0;

    gameSpeed = 15;
    objectDelay = 5;

    // initial ship position
    shipPosX = 2;
    shipPosY = 15;

    // init objects
    uint8_t cnt;
    for(cnt = 0; cnt < SPACEFLIGHT_NUM_OBJECTS; cnt++)
    {
        allObjects[cnt].position.x = 0;
        allObjects[cnt].position.y = 0;
        allObjects[cnt].color = WHITE;
        allObjects[cnt].type = 0;
    }

    // init shots
    for(cnt = 0; cnt < SPACEFLIGHT_NUM_SHOTS; cnt++)
    {
        allShots[cnt].x = 0;
        allShots[cnt].y = 0;
    }

    // init top bar
    // -- paint ship
    setHLine(44,45,3,SHIP_BODY_COLOR);
    setHLine(43,46,4,SHIP_BODY_COLOR);
    setHLine(43,46,5,SHIP_BODY_COLOR);
    setHLine(42,47,6,SHIP_BODY_COLOR);
    paintFilledRectangle(44,4,45,5,SHIP_COCKPIT_COLOR);
    // -- paint number of lifes
    char livesString[2];
    convertToString(lifes,livesString,2);
    writeString(livesString, 50,2, LIGHT_YELLOW);
    // -- paint number of shots
    paintNumShots();

    paintShip(SHIP_BODY_COLOR);
}

static void paintNumShots()
{
    paintFilledRectangle(35,2,39,6,SPACEFLIGHT_BACKGROUND);

    if(maxShots > 1)
    {
        setPixel(35,2,WEAPON_ITEM_COLOR);
        setPixel(39,6,WEAPON_ITEM_COLOR);
    }
    if(maxShots > 3)
    {
        setPixel(35,6,WEAPON_ITEM_COLOR);
        setPixel(39,2,WEAPON_ITEM_COLOR);
    }
    if(maxShots % 2 == 1)
    {
        setPixel(37,4,WEAPON_ITEM_COLOR);
    }
}

static void paintShip(uint8_t color)
{
    // if shield item collected, paint in different color
    if(color != SPACEFLIGHT_BACKGROUND && flags.shield == 1)
    {
        color = SHIELD_ITEM_COLOR;
    }

    // paint body
    setHLine(shipPosX,shipPosX+1,shipPosY,color);
    setHLine(shipPosX,shipPosX+3,shipPosY+1,color);
    setHLine(shipPosX,shipPosX+5,shipPosY+2,color);
    setHLine(shipPosX,shipPosX+6,shipPosY+3,color);

    // paint cockpit
    if(color != SPACEFLIGHT_BACKGROUND)
    {
        setHLine(shipPosX+2,shipPosX+3,shipPosY+1,SHIP_COCKPIT_COLOR);
        setHLine(shipPosX+2,shipPosX+4,shipPosY+2,SHIP_COCKPIT_COLOR);
    }
}

static void checkShipCollision(uint8_t* hitColor, uint8_t* hitX)
{
    // check all adjacent pixels to the ship
    uint8_t pxCnt;
    uint8_t collisionColor = SPACEFLIGHT_BACKGROUND;

    // check left side of ship
    for(pxCnt = 0; pxCnt < 6; pxCnt++)
    {
        collisionColor = getPixel(shipPosX-1,shipPosY-1+pxCnt);
        if(collisionColor != SPACEFLIGHT_BACKGROUND)
        {
            (*hitColor) = collisionColor;
            (*hitX) = shipPosX-1;
            return;
        }
    }

    // check bottom side of ship
    for(pxCnt = 0; pxCnt < 7; pxCnt++)
    {
        collisionColor = getPixel(shipPosX+pxCnt,shipPosY+4);
        if(collisionColor != SPACEFLIGHT_BACKGROUND)
        {
            (*hitColor) = collisionColor;
            (*hitX) = shipPosX+pxCnt;
            return;
        }
    }

    // check top and right side of ship
    uint8_t pxCnt2;
    for(pxCnt = 0; pxCnt < 3; pxCnt++)
    {
        for(pxCnt2 = 0; pxCnt2 < 3; pxCnt2++)
        {
            collisionColor = getPixel(shipPosX+(2*pxCnt)+pxCnt2, shipPosY-1+pxCnt);
            if(collisionColor != SPACEFLIGHT_BACKGROUND)
            {
                (*hitColor) = collisionColor;
                (*hitX) = shipPosX+(2*pxCnt)+pxCnt2;
                return;
            }
        }
    }
    collisionColor = getPixel(shipPosX+6,shipPosY+2);
    if(collisionColor != SPACEFLIGHT_BACKGROUND)
    {
        (*hitColor) = collisionColor;
        return;
    }

    // check right side of ship
    for(pxCnt = 0; pxCnt < 3; pxCnt++)
    {
        collisionColor = getPixel(shipPosX+7,shipPosY+2+pxCnt);
        if(collisionColor != SPACEFLIGHT_BACKGROUND)
        {
            (*hitColor) = collisionColor;
            (*hitX) = shipPosX+7;
            return;
        }
    }

    // nothing hit
    (*hitColor) = SPACEFLIGHT_BACKGROUND;
    (*hitX) = 0;
    return;
}

static void paintObject(Object* obj, uint8_t deleteObject)
{
    uint8_t x = (*obj).position.x;
    uint8_t y = (*obj).position.y;
    uint8_t color;

    if(deleteObject)
    {
        color = SPACEFLIGHT_BACKGROUND;
    }
    else
    {
        color = (*obj).color;
    }

    if((*obj).type <= 1 || (*obj).type == 5 || (*obj).type == 6) // big stone, one piece
    {
        setHLine(x+1,x+2,y,color);
        setHLine(x,x+3,y+1,color);
        setHLine(x,x+3,y+2,color);
        setHLine(x+1,x+2,y+3,color);
    }
    else if((*obj).type <= 4) // type == 2: two small pieces, 3,4: only one of the pieces left
    {
        if((*obj).type == 2 || (*obj).type == 3)
        {
            setHLine(x,x+1,y-1,color);
            setHLine(x,x+1,y,color);
        }
        if((*obj).type == 2 || (*obj).type == 4)
        {
            setHLine(x,x+1,y+3,color);
            setHLine(x,x+1,y+4,color);
        }
    }
    else if((*obj).type == 8) // weapon or shield item
    {
        setHLine(x,x+1,y+1,color);
        setHLine(x,x+1,y+2,color);
    }
}


static void handleInputs()
{
    // pause and unpause game
    if(isPushed(BUTTON_ZERO))
    {
        flags.paused = !flags.paused;
        checkForReset(); // see game.h
    }

    if(!flags.paused)
    {
        if(isPushed(BUTTON_LEFT))
        {
            paintShip(SPACEFLIGHT_BACKGROUND);
            if(shipPosX > 1) shipPosX--;
            paintShip(SHIP_BODY_COLOR);
        }
        else if(isPushed(BUTTON_RIGHT))
        {
            paintShip(SPACEFLIGHT_BACKGROUND);
            if(shipPosX < COLUMNS - 8) shipPosX++; // minus: ship width
            paintShip(SHIP_BODY_COLOR);
        }
        else if(isPushed(BUTTON_UP))
        {
            paintShip(SPACEFLIGHT_BACKGROUND);
            if(shipPosY > MIN_Y_POS) shipPosY--;
            paintShip(SHIP_BODY_COLOR);
        }
        else if(isPushed(BUTTON_DOWN))
        {
            paintShip(SPACEFLIGHT_BACKGROUND);
            if(shipPosY < LINES - 5) shipPosY++; // minus: ship height
            paintShip(SHIP_BODY_COLOR);
        }
        else if(isPushed(BUTTON_ONE)) // shot
        {
            uint8_t cnt;
            for(cnt = 0; cnt < maxShots; cnt++)
            {
                // if a shot can be made: shoot
                if(allShots[cnt].x == 0 && allShots[cnt].y == 0 && score > 0)
                {
					playSong(KLICK2);
                    score--;
                    allShots[cnt].x = shipPosX+7;
                    allShots[cnt].y = shipPosY+3;
                    break;
                }
            }
        }
    }
}

static void repaint()
{
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

        writeString(oldScoreString,2,2,SPACEFLIGHT_BACKGROUND); // delete old score
        writeString(scoreString,2,2,LIGHT_YELLOW); // paint new score
        oldScore = score;
    }
}


static void handleShipCollision()
{
    if(flags.shipCollision == 3)
    {
        // ship hit a weapon item
        if(maxShots < SPACEFLIGHT_NUM_SHOTS)
        {
            maxShots++;
            paintNumShots();
        }
        else
        {
            score += 10;
        }
        // reset collision flag
        flags.shipCollision = 0;
    }
    else if(flags.shipCollision == 2)
    {
        flags.shield = 1;
        flags.shipCollision = 0;
    }
    else if(flags.shipCollision == 1)
    {
        // paint an animated explosion
        paintShip(SPACEFLIGHT_BACKGROUND);
        shipPosX += 4;
        shipPosY += 2;

        uint8_t myColors[6] = {ORANGE, ORANGE, ORANGE, RED, PINK, SPACEFLIGHT_BACKGROUND};
        uint8_t cnt;
        for(cnt = 0; cnt < 6; cnt++)
        {
            if(cnt == 1)
            {
                setPixel(shipPosX-1,shipPosY-1,ORANGE);
                setPixel(shipPosX+2,shipPosY-1,ORANGE);
                setPixel(shipPosX-1,shipPosY+2,ORANGE);
                setPixel(shipPosX+2,shipPosY+2,ORANGE);
            }

            if(cnt > 1)
            {
                setPixel(shipPosX-2,shipPosY-2,myColors[cnt]);
                setPixel(shipPosX+3,shipPosY-2,myColors[cnt]);
                setPixel(shipPosX-2,shipPosY+3,myColors[cnt]);
                setPixel(shipPosX+3,shipPosY+3,myColors[cnt]);

                if(cnt < 5)
                {
                    paintRectangle(shipPosX-1,shipPosY-1,shipPosX+2,shipPosY+2,myColors[cnt+1]);
                }
            }

            if(cnt < 4)
            {
                paintRectangle(shipPosX,shipPosY,shipPosX+1,shipPosY+1,myColors[cnt+2]);
            }
            wait(200);
        }

        //         the loop above is equivalent to this:
        //
        //        paintRectangle(shipPosX,shipPosY,shipPosX+1,shipPosY+1,ORANGE);
        //        wait(200);
        //        setPixel(shipPosX-1,shipPosY-1,ORANGE);
        //        setPixel(shipPosX+2,shipPosY-1,ORANGE);
        //        setPixel(shipPosX-1,shipPosY+2,ORANGE);
        //        setPixel(shipPosX+2,shipPosY+2,ORANGE);
        //        paintRectangle(shipPosX,shipPosY,shipPosX+1,shipPosY+1,RED);
        //        wait(200);
        //        setPixel(shipPosX-2,shipPosY-2,ORANGE);
        //        setPixel(shipPosX+3,shipPosY-2,ORANGE);
        //        setPixel(shipPosX-2,shipPosY+3,ORANGE);
        //        setPixel(shipPosX+3,shipPosY+3,ORANGE);
        //        paintRectangle(shipPosX-1,shipPosY-1,shipPosX+2,shipPosY+2,RED);
        //        paintRectangle(shipPosX,shipPosY,shipPosX+1,shipPosY+1,PINK);
        //        wait(200);
        //        setPixel(shipPosX-2,shipPosY-2,RED);
        //        setPixel(shipPosX+3,shipPosY-2,RED);
        //        setPixel(shipPosX-2,shipPosY+3,RED);
        //        setPixel(shipPosX+3,shipPosY+3,RED);
        //        paintRectangle(shipPosX-1,shipPosY-1,shipPosX+2,shipPosY+2,PINK);
        //        paintRectangle(shipPosX,shipPosY,shipPosX+1,shipPosY+1,SPACEFLIGHT_BACKGROUND);
        //        wait(200);
        //        setPixel(shipPosX-2,shipPosY-2,PINK);
        //        setPixel(shipPosX+3,shipPosY-2,PINK);
        //        setPixel(shipPosX-2,shipPosY+3,PINK);
        //        setPixel(shipPosX+3,shipPosY+3,PINK);
        //        paintRectangle(shipPosX-1,shipPosY-1,shipPosX+2,shipPosY+2,SPACEFLIGHT_BACKGROUND);
        //        wait(200);
        //        setPixel(shipPosX-2,shipPosY-2,SPACEFLIGHT_BACKGROUND);
        //        setPixel(shipPosX+3,shipPosY-2,SPACEFLIGHT_BACKGROUND);
        //        setPixel(shipPosX-2,shipPosY+3,SPACEFLIGHT_BACKGROUND);
        //        setPixel(shipPosX+3,shipPosY+3,SPACEFLIGHT_BACKGROUND);

        wait(2000);

        lifes--; // lose one life
        if(maxShots > 1)
        {
            maxShots--; // lose one shot
        }

        if(lifes == 255) // 255 == -1 as unsigned integer
        {
            // game over
            flags.gameOver = 1;
        }
        else
        {
            // reset game
            initGame(1);
        }
    }
}


// handles objects touched by the ship
static void handleTouchedObject(uint8_t hitX)
{
    // find out which object was touched
    uint8_t cnt;
    for(cnt = 0; cnt < SPACEFLIGHT_NUM_OBJECTS; cnt++)
    {
        uint8_t checkObjPos = allObjects[cnt].position.x;
        if(absi(checkObjPos - hitX) < 4)
        {
            // delete old object type
            paintObject(&allObjects[cnt], 1);
            allObjects[cnt].position.x = 0;
            allObjects[cnt].position.y = 0;
            break;
        }
    }
}

// handles objects hit by a bullet
static void handleHitObject(Point2D* shotPos)
{
    // find out which object was hit
    uint8_t cnt;
    for(cnt = 0; cnt < SPACEFLIGHT_NUM_OBJECTS; cnt++)
    {
        uint8_t checkObjPos = allObjects[cnt].position.x;
        if(checkObjPos == (*shotPos).x || checkObjPos+1 == (*shotPos).x || checkObjPos+2 == (*shotPos).x)
        {
			// TODO: sound for hit object

            // delete old object type
            paintObject(&allObjects[cnt], 1);
            // assign new object type
            switch(allObjects[cnt].type)
            {
            case 0: // unhit stone
                allObjects[cnt].color = WHITE; //TODO: GRAY;
                allObjects[cnt].type = 1;
                break;
            case 1: // stone was hit once
                {
                    uint8_t limit = maxShots < 3 ? 5 : 3; // it is more likely to get an item if one has only 1 or 2 shots
                    if(getRandomNumber() % 16 < limit)
                    {
                        // let an item appear
                        allObjects[cnt].color = WEAPON_ITEM_COLOR;
                        allObjects[cnt].type = 8;
                    }
                    else
                    {
                        // let the stone split
                        allObjects[cnt].color = WHITE;
                        allObjects[cnt].type = 2;
                    }
                }
                score += 5;
                break;
            case 2: // stone broke into 2 pieces
                if((*shotPos).y <= allObjects[cnt].position.y) // upper piece was hit
                {
                    allObjects[cnt].type = 4;
                }
                else // lower piece was hit
                {
                    allObjects[cnt].type = 3;
                }
                break;
            case 3: // only upper piece left
            case 4: // only lower piece left
                allObjects[cnt].position.x = 0;
                allObjects[cnt].position.y = 0;
                score += 10;
                break;
            case 5: // moving object: first hit
                allObjects[cnt].type = 6;
                score += 5;
                break;
            case 6: // moving object: second hit
                allObjects[cnt].type = 7;
                allObjects[cnt].color = WHITE;
                allObjects[cnt].moveDown = 0;
                allObjects[cnt].moveUp = 0;

                if(getRandomNumber() % 16 < 4 && flags.shield == 0)
                {
                    // let an item appear
                    allObjects[cnt].color = SHIELD_ITEM_COLOR;
                    allObjects[cnt].type = 8;
                }
                else
                {
                    allObjects[cnt].type = 0;
                    score += 5;
                }
                break;
            case 8: // item was hit
                allObjects[cnt].position.x = 0;
                allObjects[cnt].position.y = 0;
                break;
            }
            break; // break for-loop
        }
    }
    // reset bullet
    (*shotPos).x = 0;
    (*shotPos).y = 0;
}

static void computeNextStep()
{
    // slows down calculation of next step
    if(update++ % UPDATE_LIMIT == 0)
    {
        // faster updates than speed game (e.g. for shooting)
        if(update2 == 0 || update2 == gameSpeed/4 || update2 == gameSpeed/2 || update2 == gameSpeed/2+gameSpeed/4)
        {
            // check if one of the bullets hit something
            uint8_t shotCnt;
            for(shotCnt = 0; shotCnt < maxShots; shotCnt++)
            {
                Point2D shotPos = allShots[shotCnt];
                // if a shot has been fired
                if(shotPos.x != 0 || shotPos.y != 0)
                {
                    setPixel(shotPos.x, shotPos.y, SPACEFLIGHT_BACKGROUND);
                    shotPos.x++;
                    // if end of screen has not been reached
                    if(shotPos.x < COLUMNS)
                    {
                        // if no collision of shot with obstacle (e.g. stone)
                        if(getPixel(shotPos.x, shotPos.y) == SPACEFLIGHT_BACKGROUND)
                        {
                            setPixel(shotPos.x, shotPos.y, WEAPON_ITEM_COLOR);
                        }
                        else // collision of shot and obstacle
                        {
                            handleHitObject(&shotPos);
                        }
                    }
                    else // end of screen reached: reset bullet
                    {
                        shotPos.x = 0;
                        shotPos.y = 0;
                    }
                }
                allShots[shotCnt] = shotPos;
            }
        }


        // next step only computed every gameSpeed-th time the update reaches UPDATE_LIMIT
        if(update2++ == gameSpeed)
        {
            update2 = 0;

            uint8_t objCnt;
            // randomly create new objects if last object was created minimum objectDelay steps ago
            if(objectDelay == 0 && getRandomNumber() % 4 == 0)
            {
                for(objCnt = 0; objCnt < SPACEFLIGHT_NUM_OBJECTS; objCnt++)
                {
                    // empty storage position found: create new object
                    if(allObjects[objCnt].position.x == 0 && allObjects[objCnt].position.y == 0)
                    {
                        allObjects[objCnt].position.x = COLUMNS-4;
                        allObjects[objCnt].position.y = getRandomNumber() % (LINES - 4 - MIN_Y_POS) + MIN_Y_POS;

                        // number of step to delay next object
                        objectDelay = 8;

                        // determine which object to create: stone or ship
                        uint8_t createStone = 1;
                        if(score < 100)
                        {
                            createStone = 1;
                        }
                        else if(score < 250)
                        {
                            if(getRandomNumber() % 8 == 0) createStone = 0;
                            objectDelay -= 1;
                        }
                        else if(score < 500)
                        {
                            if(getRandomNumber() % 4 == 0) createStone = 0;
                            objectDelay -= 2;
                        }
                        else
                        {
                            if(getRandomNumber() % 2 == 0) createStone = 0;
                            objectDelay -= 3;
                        }

                        // create object
                        if(createStone == 1)
                        {
                            allObjects[objCnt].color = WHITE;
                            allObjects[objCnt].type = 0; // stone
                        }
                        else
                        {
                            allObjects[objCnt].color = KHAKI;
                            allObjects[objCnt].type = 5;
                            allObjects[objCnt].moveDown = 1; // vertically moving object
                        }
                        break;
                    }
                }
            }
            else // decrease object delay if no object was created
            {
                if(objectDelay > 0)
                {
                    objectDelay--;
                }
            }

            // move objects
            for(objCnt = 0; objCnt < SPACEFLIGHT_NUM_OBJECTS; objCnt++)
            {
                if(allObjects[objCnt].position.x != 0 || allObjects[objCnt].position.y != 0)
                {
                    // delete at old position
                    paintObject(&allObjects[objCnt], 1);
                    // advance one step
                    allObjects[objCnt].position.x--;
                    // vertically moving object (types 5 to 6)
                    if(allObjects[objCnt].type >= 5 && allObjects[objCnt].type <= 6)
                    {
                        if(allObjects[objCnt].moveDown == 1)
                        {
                            if(allObjects[objCnt].position.y < LINES - 8) allObjects[objCnt].position.y++;
                            else
                            {
                                allObjects[objCnt].moveDown = 0;
                                allObjects[objCnt].moveUp = 1;
                            }
                        }
                        else if(allObjects[objCnt].moveUp == 1)
                        {
                            if(allObjects[objCnt].position.y > MIN_Y_POS) allObjects[objCnt].position.y--;
                            else
                            {
                                allObjects[objCnt].moveDown = 1;
                                allObjects[objCnt].moveUp = 0;
                            }
                        }
                    }
                    if(allObjects[objCnt].position.x == 255) // 8 bit unsigned value: 255 = -1
                    {
                        // set to invalid position and don't paint
                        allObjects[objCnt].position.x = 0;
                        allObjects[objCnt].position.y = 0;
                        oldScore = score;
                        score++;
                    }
                    else
                    {
                        // paint at new position
                        paintObject(&allObjects[objCnt],0);
                    }
                }
            }

            // check collisions with ship
            uint8_t hitColor;
            uint8_t hitX = 0;
            checkShipCollision(&hitColor, &hitX);

            if(hitColor != SPACEFLIGHT_BACKGROUND)
            {
                if(hitColor == WEAPON_ITEM_COLOR || hitColor == SHIELD_ITEM_COLOR) // item collected
                {
					playSong(KLICK1);
                    // flag = 2 for shield, flag = 3 for weapon
                    flags.shipCollision = hitColor == WEAPON_ITEM_COLOR ? 3 : 2;
                    // find the object that was collected and reset it
                    handleTouchedObject(hitX);
                }
                else // hit obstacle
                {
                    if(flags.shield == 1)
                    {
						playSong(KLICK2);
                        // no damage done with shield
                        flags.shield = 0;
                        // find the object that was collected and reset it
                        handleTouchedObject(hitX);
                    }
                    else
                    {
                        flags.shipCollision = 1;
						playSong(EXPLODE);
                    }
                }
                handleShipCollision();
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

    showHighScoreMenu(score); // TODO
}


void spaceFlight_startGame()
{
    showGameTitle("SPACE FLIGHT", MINT, BLACK, 3, 15);
    // set myOP1 to a value between 1 and 3, default 1 and set myOP2 to a value between 1 and 9, default 3
    // showOptionsMenu("myOP1__myOP2",14,15, &myOP1, 1, 3, 1, &myOP2, 1, 9, 3);
    initGame(0);
    while(1)
    {
        do
        {
            handleInputs();
        }
        while(flags.paused);
        repaint();
        computeNextStep();
        if(flags.gameOver) break;
        wait(1);// only for the simulator
    }
    handleGameOver();
}
