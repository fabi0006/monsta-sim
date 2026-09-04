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


#ifndef MONSTADEFS_H
#define MONSTADEFS_H

#ifdef __cplusplus
extern "C" {
#endif


/*
*   This is an interface to the MONSTA microcontroller c programm
*/

// typedefs for the simulator
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;   // VS: changed long int to int
typedef signed char int8_t;
typedef short int int16_t;

void reset();

// ------------------------------------------------------------------------>> sound.h

#define TIMER_FREQ	2500000

#define HZ_50		50000
#define HZ_100		25000
#define HZ_200		12500
#define HZ_500		5000
#define HZ_1000		2500

#define C4		(TIMER_FREQ/262)
#define Cx4		(TIMER_FREQ/277)
#define D4		(TIMER_FREQ/294)
#define E4		(TIMER_FREQ/330)
#define F4		(TIMER_FREQ/349)
#define Fx4		(TIMER_FREQ/370)
#define G4		(TIMER_FREQ/392)
#define Gx4		(TIMER_FREQ/415)
#define A4		(TIMER_FREQ/440)
#define Ax4		(TIMER_FREQ/466)
#define B4		(TIMER_FREQ/494)
#define C5		(TIMER_FREQ/523)

#define SHORT_HIGH_BEEP 	0
#define TONLEITER 			1
#define ENTCHEN				2
#define ZONK				3
#define SCORE				4

#define TEST2				5
#define EXPLODE				6
#define TEST4				7
#define TEST5				8

#define PIANO				9
#define SHORT_VERY_HIGH_BEEP 10
#define KLICK1				11
#define KLICK2				12


void initSound();
void playSong(uint8_t num);
void stopSong();

// sound.h <<------------------------------------------------------------------------


// ------------------------------------------------------------------------>> graphics.h
#define LINES 			42
#define COLUMNS 		56

struct Point2D
{
    uint8_t x;
    uint8_t y;
};
typedef struct Point2D Point2D;

enum colors
{
    BLACK = 0,
    BLUE = 17,
    RED = 34,
    MAGENTA = 51,
    GREEN = 68,
    CYAN = 85,
    YELLOW = 102,
    WHITE = 119,

    DARK_BLUE = 1,
    DARK_RED = 2,
    DARK_MAGENTA = 18,
    DARK_GREEN = 4,
    DARK_CYAN = 20,
    DARK_YELLOW = 36,
    DARK_WHITE = 52,

    LIGHT_BLUE = 53,
    LIGHT_RED = 54,
    LIGHT_MAGENTA = 55,
    LIGHT_GREEN = 86,
    LIGHT_CYAN = 87,
    LIGHT_YELLOW = 103,

    PALE_VIOLET = 19,
    SEA_BLUE = 21,
    MINT = 69,
    GRASS_GREEN = 70,
    PINK = 35,
    ORANGE = 38,

    // convenient names for some colors
    VIOLET = DARK_MAGENTA,
    FRESH = LIGHT_GREEN,
    KHAKI = DARK_YELLOW,
    GRAY = DARK_WHITE
};

uint8_t composeColor(uint8_t odd, uint8_t even);
void setPixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t getPixel(uint8_t x, uint8_t y);
void setHLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color);
void setCompleteHLine(uint8_t y, uint8_t color);
void setVLine(uint8_t x, uint8_t y1, uint8_t y2, uint8_t color);
void setCompleteVLine(uint8_t x, uint8_t color);
void setScreen(uint8_t color);
void paintRectangle(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t color);
void paintFilledRectangle(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t color);
void paintBorders(uint8_t color);
uint16_t getTimestamp();

// graphics.h <<------------------------------------------------------------------------



// ---------------------------------------------------------------------->> graphics3D.h
struct Point3D
{
    float x;
    float y;
    float z;
    uint8_t color;
};
typedef struct Point3D Point3D;


struct Point4D
{
    float x;
    float y;
    float z;
    float w;
    uint8_t color;
};
typedef struct Point4D Point4D;


struct Polygon
{
    uint8_t size;
    Point3D* points;
    uint8_t color;
};
typedef struct Polygon Polygon;


struct Matrix4D
{
    Point4D line1;
    Point4D line2;
    Point4D line3;
    Point4D line4;
};
typedef struct Matrix4D Matrix4D;

void drawLine(uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by, uint8_t color);
// graphics3D.h <<----------------------------------------------------------------------



// ------------------------------------------------------------------------>> gamePad.h

#define BUTTON_UP		0
#define BUTTON_RIGHT            1
#define BUTTON_DOWN		2
#define BUTTON_LEFT		3
#define BUTTON_ZERO		4
#define BUTTON_ONE		5
#define BUTTON_TWO		6

#define BUTTON_ANY		8
#define ANY_BUTTON		BUTTON_ANY

uint8_t isPushed(uint8_t button);

// gamePad.h <<------------------------------------------------------------------------


// ------------------------------------------------------------------------>> symbols.h

// bit patterns
#define LETTER_A	0x699F9000
#define LETTER_B	0xE9E9E000
#define LETTER_C	0x72460000
#define LETTER_D	0xE999E000
#define LETTER_E	0xF34E0000
#define LETTER_F	0xF3480000
#define LETTER_G	0x78B97000
#define LETTER_H	0x99F99000
#define LETTER_I	0xE92E0000
#define LETTER_J	0x11196000
#define LETTER_K	0x9ACA9000
#define LETTER_L	0x924E0000
#define LETTER_M	0x8EEB1880
#define LETTER_N	0x9DB99000
#define LETTER_O	0x69996000
#define LETTER_P	0xE9E88000
#define LETTER_Q	0x69996100
#define LETTER_R	0xE99E9000
#define LETTER_S	0x7861E000
#define LETTER_T	0xE9240000
#define LETTER_U	0x99996000
#define LETTER_V	0x99AA4000
#define LETTER_W	0x8D6B5500
#define LETTER_X	0xB55A0000
#define LETTER_Y	0x99716000
#define LETTER_Z	0xE54E0000

#define DIGIT_0		0x69996000
#define DIGIT_1		0xD5400000
#define DIGIT_2		0xE168F000
#define DIGIT_3		0xE161E000
#define DIGIT_4		0x26AF2000
#define DIGIT_5		0xF861E000
#define DIGIT_6		0x68E96000
#define DIGIT_7		0xF1244000
#define DIGIT_8		0x69696000
#define DIGIT_9		0x69716000

#define UNKNOWN_CHAR	0xFFFFF000
#define CHAR_PERIOD		0x08000000
#define CHAR_COLON		0x50000000
#define CHAR_EXCLM		0xE8000000

void writeBitPattern(uint32_t pattern, uint8_t cols, uint8_t rows, uint8_t x, uint8_t y, uint8_t color);
void paintHeart(uint8_t x, uint8_t y, uint8_t color);
void paintMonstaLogo(uint8_t x, uint8_t y, uint8_t color);
void writeString(char* word, uint8_t x, uint8_t y, uint8_t color);

// symbols.h <<------------------------------------------------------------------------


// ------------------------------------------------------------------------>> util.h
void convertToString(uint16_t num, char* str, uint8_t length);
uint16_t absi(int16_t value);
uint8_t getRandomNumber();
void wait(uint16_t msec);
void startMonsta();

// util.h <<------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // MONSTADEFS_H
