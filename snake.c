/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    snake.c
 *
 * Description:
 *    Implements the snake game.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <stdlib.h>
#include "lcd.h"
#include "key.h"
#include "select.h"

#include <lpc2xxx.h>
#include <consol.h>
#include "i2c.h"
#include "adc.h"

#include "snake.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define MAX_CARS 18
#define MAX_LANES 8
#define MAX_ROWS 10
#define LANE_HEIGHT 12
#define GRID_WIDTH  12
#define FROGGER_PADDING  2
#define FROGGER_WIDTH  8
#define LEFT_PADDING 4
#define TOP_INTERFACE_HEIGHT 16

#define INITIAL_FROGGER_X 5
#define INITIAL_FROGGER_Y 7

#define COLOR_YELLOW 0xfc
#define COLOR_RED 0xe0
#define COLOR_GREEN 0x1c
#define COLOR_BLUE 0x03 
#define COLOR_PURPLE 0xe3

#define SEVEN_SEG_A   0x00010000
#define SEVEN_SEG_B   0x00020000
#define SEVEN_SEG_C   0x00040000
#define SEVEN_SEG_D   0x04000000
#define SEVEN_SEG_E   0x08000000
#define SEVEN_SEG_F   0x10000000
#define SEVEN_SEG_G   0x20000000
#define SEVEN_SEG_DOT 0x40000000

int collides(tU8 laneX, tU8 laneY);

void drawFrogger(int black);

void set7seg(tU8 value);

void updateFrogger(tU8 key);

void lightLeds();

void darkenLeds();

void loseLife();

void saveScore(int newScore);


enum CarType { SLOWEST, SLOW, NORMAL, FAST, FASTEST, CAR_TYPE_COUNT };
int topPaddings[] = {2, 2, 2, 2, 2};
int widths[] = {32, 28, 24, 20, 16};
tU8 colors[] = {COLOR_YELLOW, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_PURPLE};
int speeds[] = {1, -2, 3, -4, 5};

enum CarType carTypesOnLanes[] = { NORMAL, FAST, FASTEST, SLOW, SLOWEST, FAST };

struct Car
{
  tS32 x;
  tU8 lane;
  enum CarType type;
} cars[MAX_CARS];

/* Frogger stuff */
static tU8 froggerX = INITIAL_FROGGER_X, froggerY = INITIAL_FROGGER_Y;


/*****************************************************************************
 * Local prototypes
 ****************************************************************************/
static void setupLevel();
static void gotoxy(tU8 x, tU8 y, tU8 color);

static void drawCar(struct Car* car, int black);
static void updateCar(struct Car* car);

int collides(tU8 laneX, tU8 laneY);

void set7seg(tU8 value);
void lightLeds();
void darkenLeds();
void loseLife();

static void nextLevel();


/*****************************************************************************
 * Local variables
 ****************************************************************************/
static tS32  level;
static tBool firstPress;
static tS32  score = 0, high_score = 0;
static tS8   direction = KEY_RIGHT;
static tS8   lives = 7;

/*****************************************************************************
 * External variables
 ****************************************************************************/
extern volatile tU32 ms;


/*****************************************************************************
 *
 * Description:
 *    Implement Snake game
 *
 ****************************************************************************/
void playSnake(void)
{
  tU8 keypress, lastKeypress = KEY_NOTHING;
  tU8 done = FALSE;

  //game loop
  do
  {
    level     = 1;
    srand(ms);        //Ensure random seed initiated
    setupLevel();

    lives = -1;
    lightLeds();

    set7seg(1);

    //main loop
    do
    {
      tS32 i;
      
      //delay between snake moves
      //osSleep(speed * PAUSE_LENGTH);

      osSleep(14 * 2);
      
      //check if key press
      keypress = checkKey();
      
      // draw car at new position
      for (i = 0; i < MAX_CARS; ++i) {
        drawCar(&cars[i], 1);
    	updateCar(&cars[i]);
        drawCar(&cars[i], 0);
	  }

	  drawFrogger(1);
	  updateFrogger(keypress);
	  drawFrogger(0);

	  if (collides(froggerX, froggerY)) {
        froggerX = INITIAL_FROGGER_X;
        froggerY = INITIAL_FROGGER_Y;

        loseLife();
	  }

      if (froggerY <= 0) {
        nextLevel();
      }

    } while (keypress != KEY_CENTER && level < 10 && lives >= 1);

    darkenLeds();
    set7seg(0);

    saveScore(level - 1);
    
    //game over message
    if (score > high_score)
      high_score = score;

    {
      tMenu menu;
        
      menu.xPos = 10;
      menu.yPos = 40;
      menu.xLen = 6+(12*8);
      menu.yLen = 4*14;
      menu.noOfChoices = 2;
      menu.initialChoice = 0;
      menu.pHeaderText = level >= 10? "You Win!!!": "Game over!";
      menu.headerTextXpos = 20;
      menu.pChoice[0] = "Restart game";
      menu.pChoice[1] = "End game";
      menu.bgColor       = 0;
      menu.borderColor   = 0x6d;
      menu.headerColor   = 0;
      menu.choicesColor  = 0xfd;
      menu.selectedColor = 0xe0;
        
      switch(drawMenu(menu))
      {
        case 0: done = FALSE; break;  //Restart game
        case 1: done = TRUE; break;   //End game
        default: break;
      }
    }

  } while (done == FALSE);
}


/*****************************************************************************
 *
 * Description:
 *    Initialize one level of the game. Draw game board.
 *
 ****************************************************************************/
void setupLevel()
{
  tS32 row, col, i;

  //clear screen
  lcdColor(0,0xe0);
  lcdClrscr();

  //draw frame
  lcdGotoxy(22,0);
  lcdPuts("FROGGER 2000");
  
  direction   = KEY_RIGHT;
  firstPress  = TRUE;

  tU8 carsPerLane = MAX_CARS / (MAX_LANES - 2);

  for (i = 0; i < MAX_CARS; ++i) {
	  cars[i].lane = i / carsPerLane + 1;
	  cars[i].type = carTypesOnLanes[cars[i].lane - 1];
	  cars[i].x = (cars[i].lane * carsPerLane - i) * widths[cars[i].type] + (24 + 4 * abs(speeds[cars[i].type]));
  }
}

/*****************************************************************************
 *
 * Description:
 *    Goto a specifc xy position and draw a 4x4 pixel rectangle in
 *    specified color
 *
 ****************************************************************************/
void gotoxy(tU8 x, tU8 y, tU8 color)
{
  lcdRect(2+(x*4), 16+(y*4), 4, 4, color);
}

void drawCar(struct Car* car, int black)
{
  lcdRect(car->x,
		  TOP_INTERFACE_HEIGHT + (car->lane * LANE_HEIGHT) + topPaddings[car->type],
		  widths[car->type],
		  LANE_HEIGHT - topPaddings[car->type] * 2,
		  black ? 0x00 : colors[car->type]);
}

void updateCar(struct Car* car)
{
  if (car->x > 128 + 50) car->x = -50;
  else if (car->x < -50) car->x = 128 + 50;

  car->x += speeds[car->type];
}

void drawFrogger(int black) {
  lcdRect(froggerX * GRID_WIDTH + FROGGER_PADDING + LEFT_PADDING,
		  TOP_INTERFACE_HEIGHT + (froggerY * LANE_HEIGHT) + FROGGER_PADDING,
		  FROGGER_WIDTH,
		  FROGGER_WIDTH,
		  black ? 0x00 : COLOR_GREEN);
}

void updateFrogger(tU8 key) {
	switch (key) {
		case KEY_UP:
			if (froggerY > 0) froggerY -= 1;
		break;

		case KEY_DOWN:
			if (froggerY < MAX_LANES - 1) froggerY += 1;
		break;

		case KEY_LEFT:
			if (froggerX > 0) froggerX -= 1;
		break;

		case KEY_RIGHT:
			if (froggerX < MAX_ROWS - 1) froggerX += 1;
		break;

		case KEY_NOTHING: default: break;
	}
}

int collides(tU8 laneX, tU8 laneY) {
	int froggerLeftBorder = laneX * LANE_HEIGHT;
	int froggerRightBorder = laneX * LANE_HEIGHT + LANE_HEIGHT;

	int i;
	for(i = 0; i < MAX_CARS; i++){
		struct Car car = cars[i];
		//if lane, check x precisely
		if(laneY == car.lane){
			int width = widths[i];
			int carLeftBorder = car.x;
			int carRightBorder = car.x * width;
			if(froggerRightBorder > carLeftBorder && froggerLeftBorder < carRightBorder){
				return 1;
			}
		}
	}
	return 0;
}

void nextLevel() {
    froggerX = INITIAL_FROGGER_X;
    froggerY = INITIAL_FROGGER_Y;

    int i;

    for (i = 0; i < CAR_TYPE_COUNT; ++i) {
        speeds[i] += speeds[i] > 0? 1: -1;
    }

    //score += level;

    level += 1;
    if (level < 10)
    {
        set7seg(level);
    }
    // TODO: increment level counter
    // if level == max then you win
}

void lightLeds()
{
	int i = 0;
	for (i = lives + 1; i < 8; i++)
		setPca9532Pin(i, 0);
	lives = 7;
}

void darkenLeds()
{
	int i = 0;
	for (i = lives; i >= 0; i--)
		setPca9532Pin(i, 1);
	lives = 0;
}

void loseLife() {
	if (lives != -1)
	setPca9532Pin(lives, 1);
	if (lives >= 0)
		lives -= 1;
}

void set7seg(tU8 value)
{
	IODIR |= 0x7c070000;  //7-segment

	IOSET = 0x7c070000;
  switch(value)
  {
    case  0: IOCLR = (SEVEN_SEG_A | SEVEN_SEG_B | SEVEN_SEG_C | SEVEN_SEG_D | SEVEN_SEG_E | SEVEN_SEG_F              ); break;
    case  1: IOCLR = (              SEVEN_SEG_B | SEVEN_SEG_C                                                        ); break;
    case  2: IOCLR = (SEVEN_SEG_A | SEVEN_SEG_B               | SEVEN_SEG_D | SEVEN_SEG_E               | SEVEN_SEG_G); break;
    case  3: IOCLR = (SEVEN_SEG_A | SEVEN_SEG_B | SEVEN_SEG_C | SEVEN_SEG_D                             | SEVEN_SEG_G); break;
    case  4: IOCLR = (              SEVEN_SEG_B | SEVEN_SEG_C                             | SEVEN_SEG_F | SEVEN_SEG_G); break;
    case  5: IOCLR = (SEVEN_SEG_A               | SEVEN_SEG_C | SEVEN_SEG_D               | SEVEN_SEG_F | SEVEN_SEG_G); break;
    case  6: IOCLR = (SEVEN_SEG_A               | SEVEN_SEG_C | SEVEN_SEG_D | SEVEN_SEG_E | SEVEN_SEG_F | SEVEN_SEG_G); break;
    case  7: IOCLR = (SEVEN_SEG_A | SEVEN_SEG_B | SEVEN_SEG_C                                                        ); break;
    case  8: IOCLR = (SEVEN_SEG_A | SEVEN_SEG_B | SEVEN_SEG_C | SEVEN_SEG_D | SEVEN_SEG_E | SEVEN_SEG_F | SEVEN_SEG_G); break;
    case  9: IOCLR = (SEVEN_SEG_A | SEVEN_SEG_B | SEVEN_SEG_C | SEVEN_SEG_D               | SEVEN_SEG_F | SEVEN_SEG_G); break;
    default: break;
  }

  IOSET  = 0x00006000;
}

void setNick(char* nick, char* what) {
    nick[0] = what[0];
    nick[1] = what[1];
    nick[2] = what[2];
}

void saveScore(int newScore)
{
    scores_t* temp;
    
    temp = loadScores();

    int maxpos = -1;
    int otherValue = 0;

    int i;
    for (i = 3; i >= 0; i -= 1)
    {
        switch (i)
        {
            case 0:
                otherValue = temp->score1;
                break;
            case 1:
                otherValue = temp->score2;
                break;
            case 2:
                otherValue = temp->score3;
                break;
            case 3:
                otherValue = temp->score4;
                break;
        }

        if (newScore > otherValue)
            maxpos = i;
    }

    if (maxpos != -1)
    {
        for (i = 2; i >= maxpos; i -= 1)
        {
            switch (i)
            {
                case 0:
                    temp->score2 = temp->score1;
                    temp->nick2[0] = temp->nick1[0];
                    temp->nick2[1] = temp->nick1[1];
                    temp->nick2[2] = temp->nick1[2];
                    break;
                case 1:
                    temp->score3 = temp->score2;
                    temp->nick3[0] = temp->nick2[0];
                    temp->nick3[1] = temp->nick2[1];
                    temp->nick3[2] = temp->nick2[2];
                    break;
                case 2:
                    temp->score4 = temp->score3;
                    temp->nick4[0] = temp->nick3[0];
                    temp->nick4[1] = temp->nick3[1];
                    temp->nick4[2] = temp->nick3[2];
                    break;
            }
        }

        char newNick[] = "   ";
        i = rand() % 5;

        switch (i)
        {
            case 0:
                setNick(newNick, "ABC");
                break;
            case 1:
                setNick(newNick, "ZZZ");
                break;
            case 2:
                setNick(newNick, "NOO");
                break;
            case 3:
                setNick(newNick, "DLC");
                break;
            case 4:
                setNick(newNick, "DVD");
                break;
        }

        switch (maxpos)
        {
            case 0:
                temp->score1 = newScore;
                setNick(temp->nick1, newNick);
                break;
            case 1:
                temp->score2 = newScore;
                setNick(temp->nick2, newNick);
                break;
            case 2:
                temp->score3 = newScore;
                setNick(temp->nick3, newNick);
                break;
            case 3:
                temp->score4 = newScore;
                setNick(temp->nick4, newNick);
                break;
        }
    }
}
