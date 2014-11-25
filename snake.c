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


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define MAXROW 20
#define MAXCOL 31

#define SNAKE_START_COL 15
#define SNAKE_START_ROW  7
#define PAUSE_LENGTH     2

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
static void showScore();
static void addSegment();
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
static tS32  score;
static tS32  snakeLength;
static tS32  speed;
static tS32  obstacles;
static tS32  level;
static tBool firstPress;
static tS32  high_score = 0;
static tS8   screenGrid[MAXROW][MAXCOL];
static tS8   direction = KEY_RIGHT;
static tS8   lives = 7;

struct snakeSegment
{
  tS32 row;
  tS32 col;
} snake[100];


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
    obstacles = 4;
    level     = 1;
    score     = 0;
    speed     = 14;
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
      osSleep(speed * PAUSE_LENGTH);
      
      //check if key press
      keypress = checkKey();

      //add a segment to the end of the snake
      //addSegment();

      //removed last segment of snake
      //gotoxy(snake[0].col, snake[0].row, 0);

      //remove last segment from the array
      //for(i=1; i<=snakeLength; i++)
        //snake[i-1] = snake[i];

      //display snake in yellow
      //for (i=0; i<=snakeLength; i++)
        //gotoxy(snake[i].col, snake[i].row, 0xfc);

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

      if (froggerY >= MAX_LANES - 1) {
        nextLevel();
      }


      //if first press on each level, pause until a key is pressed
      //~ if (firstPress == TRUE)
      //~ {
        //~ while(KEY_NOTHING == checkKey())
          //~ ;
        //~ firstPress = FALSE;
      //~ }

      /* collision detection - walls (bad!) */
      //~ if ((snake[snakeLength-1].row >= MAXROW) || (snake[snakeLength-1].row < 0) ||
          //~ (snake[snakeLength-1].col >= MAXCOL) || (snake[snakeLength-1].col < 0) ||

      /* collision detection - obstacles (bad!) */
          //~ (screenGrid[snake[snakeLength-1].row][snake[snakeLength-1].col] == 'x'))
        //~ keypress = KEY_CENTER;

      //collision detection - snake (bad!)
      //~ for (i=0; i<snakeLength-1; i++)
        //~ if ((snake[snakeLength-1].row) == (snake[i].row) &&
            //~ (snake[snakeLength-1].col) == (snake[i].col))
        //~ {
          //~ keypress = KEY_CENTER;   //exit loop - game over
          //~ break;
        //~ }

      //collision detection - food (good!)
      //~ if (screenGrid[snake[snakeLength-1].row][snake[snakeLength-1].col] == '.')
      //~ {
        //~ //increase score and length of snake
        //~ score += snakeLength * obstacles;
        //~ showScore();
        //~ snakeLength++;
        //~ addSegment();
//~ 
        //~ //if length of snake reaches certain size, onto next level
        //~ if (snakeLength == (level + 3) * 2)
        //~ {
          //~ score += level * 1000;
          //~ obstacles += 2;          //add obstacles
          //~ level++;
          //~ 
          //~ //check if time to inclrease speed (every 5 levels)
          //~ if ((level % 5 == 0) && (speed > 1))
            //~ speed--;
//~ 
          //~ //draw next level
          //~ setupLevel();
        //~ }
      //~ }
    } while (keypress != KEY_CENTER || level >= 10);

    darkenLeds();
    set7seg(0);
    
    //game over message
    if (score > high_score)
      high_score = score;
    showScore();

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
  lcdGotoxy(42,0);
  lcdPuts("FROGGER 2000");

  //draw game board rectangle
  //lcdRect(0, 14, (4*MAXCOL)+4, (4*MAXROW)+4, 3);
  //lcdRect(2, 16, 4*MAXCOL,     4*MAXROW,     1);

  //set up global variables for new level
  snakeLength = level + 4;
  direction   = KEY_RIGHT;
  firstPress  = TRUE;

  //fill grid with blanks
  for(row=0; row<MAXROW; row++)
    for(col=0; col<MAXCOL; col++)
      screenGrid[row][col] = ' ';

  //fill grid with Xs and food
  for(i=0; i<obstacles*2; i++)
  {
    row = rand() % MAXROW;
    col = rand() % MAXCOL;
    if (i < obstacles)
      screenGrid[row][col] = 'x';  //= obstacle
    else
      screenGrid[row][col] = '.';  //= food
  }

  //create snake array of length snakeLength
  for(i=0; i<snakeLength; i++)
  {
    snake[i].row = SNAKE_START_ROW;
    snake[i].col = SNAKE_START_COL + i;
  }

  //~ //draw game board
  //~ for(row=0; row<MAXROW; row++)
  //~ {
    //~ for(col=0; col<MAXCOL; col++)
    //~ {
      //~ switch(screenGrid[row][col])
      //~ {
        //~ case ' ': gotoxy(col,row,0); break;
        //~ case '.': gotoxy(col,row,0x1c); break;
        //~ case 'x': gotoxy(col,row,0xe0); break;
        //~ default: break;
      //~ }
    //~ }
  //~ }

  tU8 carsPerLane = MAX_CARS / (MAX_LANES - 2);

  for (i = 0; i < MAX_CARS; ++i) {
	  cars[i].lane = i / carsPerLane + 1;
	  cars[i].type = carTypesOnLanes[cars[i].lane - 1];
	  cars[i].x = (cars[i].lane * carsPerLane - i) * widths[cars[i].type] + (24 + 4 * abs(speeds[cars[i].type]));
  }

  showScore();
}


/*****************************************************************************
 *
 * Description:
 *    Draw current score
 *
 ****************************************************************************/
void showScore()
{
  tU8 str[13];
  
  str[0] = 'L';
  str[1] = ':';
  str[2] = level + '0';
  str[3] = ' ';
  str[4] = 'S';
  str[5] = ':';
  str[6] = score/100000 + '0';
  str[7] = (score/10000)%10 + '0';
  str[8] = (score/1000)%10 + '0';
  str[9] = (score/100)%10 + '0';
  str[10] = (score/10)%10 + '0';
  str[11] = score%10 + '0';
  str[12] = 0;
  
  //remove leading zeroes
  if (str[6] == '0')
  {
    str[6] = ' ';
    if (str[7] == '0')
    {
      str[7] = ' ';
      if (str[8] == '0')
      {
        str[8] = ' ';
        if (str[9] == '0')
        {
          str[9] = ' ';
          if (str[10] == '0')
          {
            str[10] = ' ';
          }
        }
      }
    }
  }
  lcdGotoxy(0,114);
  lcdPuts(str);
}


/*****************************************************************************
 *
 * Description:
 *    Add one snake segment
 *
 ****************************************************************************/
void addSegment()
{
  switch(direction)
  {
    case(KEY_RIGHT): snake[snakeLength].row = snake[snakeLength-1].row;
                     snake[snakeLength].col = snake[snakeLength-1].col+1;
                     break;
    case(KEY_LEFT) : snake[snakeLength].row = snake[snakeLength-1].row;
                     snake[snakeLength].col = snake[snakeLength-1].col-1;
                     break;
    case(KEY_UP)   : snake[snakeLength].row = snake[snakeLength-1].row-1;
                     snake[snakeLength].col = snake[snakeLength-1].col;
                     break;
    case(KEY_DOWN) : snake[snakeLength].row = snake[snakeLength-1].row+1;
                     snake[snakeLength].col = snake[snakeLength-1].col;
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

    score += level;

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
