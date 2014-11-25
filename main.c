/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2008 Embedded Artists AB
 *
 * Description:
 *    Main program for Experiment Expansion Board (for LPC2103 Education Board)
 *
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "i2c.h"
#include "adc.h"
#include "lcd.h"
#include "pca9532.h"
#include "time_manager.h"

#include "snake.h"
#include "key.h"

#include "fire_0_100x40c.h"
#include "fire_1_100x40c.h"
#include "fire_2_100x40c.h"
#include "fire_3_100x40c.h"
#include "fire_4_100x40c.h"

#define forever for(;;)

/******************************************************************************
 * Defines
 *****************************************************************************/
#define SPI_SLAVE_CS 0x00002000  //pin P0.13
#define ENC_RESET    0x00001000  //pin P0.12
#define FAILSAFE_VALUE 5000

#define PROC1_STACK_SIZE 750
#define PROC2_STACK_SIZE 750
#define INIT_STACK_SIZE  400

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 pid2;
static tU16 globalKeys;

static void proc1(void* arg);
static void proc2(void* arg);
static void initProc(void* arg);


/*****************************************************************************
 * Global variables
 ****************************************************************************/
volatile tU32 ms;

//value -1 means timer is not working
//value 0 means player time is over
//value bigger than 0 means player has time left
volatile tS32 timeLeft = -1;

static tU8 contrast = 46;

typedef enum menuState {
	menu,
	scores,
	info
} menuState_t;

/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;

  osInit();
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    Draw main menu
 *
 ****************************************************************************/
static void
refreshMenu(int pos)
{
  lcdColor(0,0);
  int i;

  for (i = 0; i < 3; i++)
  {
	  lcdGotoxy(22,20+(20*i));

	  if (i == pos)
	  {
		  lcdColor(0x00,0xfd);
	  }
	  else
	  {
		  lcdColor(0x00,0xe0);
	  }

	  switch(i)
	  {
		  case 0:
			  lcdPuts("Play");
			  break;
		  case 1:
			  lcdPuts("Hi Scores");
			  break;
		  case 2:
			  lcdPuts("Info");
			  break;
	  }
  }
}

static void
drawMenu(int pos)
{
  lcdColor(0,0);
  lcdClrscr();

  lcdRect(14, 0, 102, 128, 0x6d);
  lcdRect(15, 17, 100, 110, 0);

  lcdGotoxy(48,1);
  lcdColor(0x6d,0);
  lcdPuts("FROGGER");

  refreshMenu(pos);
}

static void
drawInfo()
{
  lcdColor(0,0);
  lcdClrscr();

  lcdRect(14, 0, 102, 128, 0x6d);
  lcdRect(15, 17, 100, 110, 0);

  lcdGotoxy(48,1);
  lcdColor(0x6d,0);
  lcdPuts("INFO");

  lcdColor(0x00,0xe0);

  lcdGotoxy(22,20);
  lcdPuts("Frogger v8");
  lcdGotoxy(22,35);
  lcdPuts("Do not play");
  lcdGotoxy(22,50);
  lcdPuts("when on");
  lcdGotoxy(22,65);
  lcdPuts("fire.");
}

static void
drawScores()
{
  lcdColor(0,0);
  lcdClrscr();

  lcdRect(14, 0, 102, 128, 0x6d);
  lcdRect(15, 17, 100, 110, 0);

  lcdGotoxy(35,1);
  lcdColor(0x6d,0);
  lcdPuts("HI SCORES");


  int i;

    for (i = 0; i < 4; i++)
    {
      // draw taken place:
      lcdGotoxy(20,20+(15*i));
      lcdColor(0x00,0xab);

      char blbl[] = "1";
      blbl[0] = i + 1 + '0';

      lcdPuts(blbl);

      // draw person's initails:
  	  lcdGotoxy(30,20+(15*i));

  	  lcdColor(0x00,0xfd);

  	  switch(i)
  	  {
  		  case 0:
  			  lcdPuts("OMG");
  			  break;
  		  case 1:
  			  lcdPuts("BLU");
  			  break;
  		  case 2:
  			  lcdPuts("YES");
  			  break;
  		  case 3:
  			  lcdPuts("RED");
  			  break;
  	  }

  	  // draw person's score:
	  lcdGotoxy(70,20+(15*i));

	  lcdColor(0x00,0xfd);

	  switch(i)
	  {
		  case 0:
			  lcdPuts("5600");
			  break;
		  case 1:
			  lcdPuts("4000");
			  break;
		  case 2:
			  lcdPuts("380");
			  break;
		  case 3:
			  lcdPuts("6");
			  break;
	  }
    }
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
  static tU8 i = 0;

  menuState_t state = menu;

  printf("\n\n\n\n\n*******************************************************\n");
  printf("*                                                     *\n");
  printf("* Demo program for 'Experiment Expansion Board'       *\n");
  printf("* running on LPC2103 Education Board.                 *\n");
  printf("* - Snake game                                        *\n");
  printf("*                                                     *\n");
  printf("* (C) Embedded Artists 2014                           *\n");
  printf("*                                                     *\n");
  printf("*******************************************************\n");

  IODIR |= 0x00006000;  //P0.13/14
  IOSET  = 0x00006000;

  lcdInit();
  initKeyProc();
  drawMenu(0);
  lcdContrast(contrast);

  int pos = 0;

  while(1)
  {
    tU8 anyKey;

    anyKey = checkKey();
    if (anyKey != KEY_NOTHING)
    {
    	if (state == menu)
    	{
			//select specific function
			if (anyKey == KEY_CENTER)
			{
			  switch(pos)
				  {
					  case 0:
						  playSnake();
						  drawMenu(pos);
						  break;
					  case 1:
						  state = scores;
						  drawScores();
						  break;
					  case 2:
						  state = info;
						  drawInfo();
						  break;
				  }
			}

			if (anyKey == KEY_DOWN)
			{
			  pos+=1;
			  if (pos > 2)
				  pos = 0;
			  refreshMenu(pos);
			}

			if (anyKey == KEY_UP)
			{
			  pos-=1;
			  if (pos < 0)
				  pos = 2;
			  refreshMenu(pos);
			}

			//adjust contrast
			else if (anyKey == KEY_RIGHT)
			{
			contrast++;
			if (contrast > 127)
			  contrast = 127;
			lcdContrast(contrast);
			}
			else if (anyKey == KEY_LEFT)
			{
			if (contrast > 0)
			  contrast--;
			lcdContrast(contrast);
			}
    	}
    	else
    	{
    		state = menu;
    		drawMenu(pos);
    	}
    }

    switch(i)
    {
      case 0: lcdIcon(15, 88, 100, 40, _fire_0_100x40c[2], _fire_0_100x40c[3], &_fire_0_100x40c[4]); i++; break;
      case 1: lcdIcon(15, 88, 100, 40, _fire_1_100x40c[2], _fire_1_100x40c[3], &_fire_1_100x40c[4]); i++; break;
      case 2: lcdIcon(15, 88, 100, 40, _fire_2_100x40c[2], _fire_2_100x40c[3], &_fire_2_100x40c[4]); i++; break;
      case 3: lcdIcon(15, 88, 100, 40, _fire_3_100x40c[2], _fire_3_100x40c[3], &_fire_3_100x40c[4]); i++; break;
      case 4: lcdIcon(15, 88, 100, 40, _fire_4_100x40c[2], _fire_4_100x40c[3], &_fire_4_100x40c[4]); i=0; break;
      default: i = 0; break;
    }
    osSleep(20);
  }
}


/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc2(void* arg)
{
  tU8 pca9532Present = FALSE;

  //check if connection with PCA9532
  pca9532Present = pca9532Init();
  
	forever
	{
	  if (TRUE == pca9532Present)
	  {
#define LED_DELAY 10
	    setPca9532Pin(0, 0);
	    setPca9532Pin(15, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(1, 0);
	    setPca9532Pin(14, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(2, 0);
	    setPca9532Pin(13, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(3, 0);
	    setPca9532Pin(12, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(4, 0);
	    setPca9532Pin(11, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(5, 0);
	    setPca9532Pin(10, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(6, 0);
	    setPca9532Pin(9, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(7, 0);
	    setPca9532Pin(8, 0);
      osSleep(LED_DELAY);
	    setPca9532Pin(0, 1);
	    setPca9532Pin(1, 1);
	    setPca9532Pin(2, 1);
	    setPca9532Pin(3, 1);
	    setPca9532Pin(4, 1);
	    setPca9532Pin(5, 1);
	    setPca9532Pin(6, 1);
	    setPca9532Pin(7, 1);
	    setPca9532Pin(8, 1);
	    setPca9532Pin(9, 1);
	    setPca9532Pin(10, 1);
	    setPca9532Pin(11, 1);
	    setPca9532Pin(12, 1);
	    setPca9532Pin(13, 1);
	    setPca9532Pin(14, 1);
	    setPca9532Pin(15, 1);
      osSleep(2);
      globalKeys = ~getPca9532Pin();
    }
  }
}


/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
initProc(void* arg)
{
  tU8 error;

  eaInit();   //initialize printf
  i2cInit();  //initialize I2C
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);
  osCreateProcess(timeProcStart, proc2Stack, PROC2_STACK_SIZE, &pid2, 3, NULL, &error);
  osStartProcess(pid2, &error);

  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void
appTick(tU32 elapsedTime)
{
  ms += elapsedTime;
}
