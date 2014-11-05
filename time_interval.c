#include "time_interval.h"
#include <printf_P.h>

static tU32 whenFinish;

/*
void set_timeout(tU32 timeout){

	VICV

	whenFinish = timeout + ms;
}
*/

void wait(int delay){

		  TIMER1_TCR = 0x02;          //stop and reset timer
		  TIMER1_PR  = 0x00;          //set prescaler to zero
		  TIMER1_MR0 = 1000 * ((FOSC * PLL_MUL) / (1000000 * PBSD)) * delay;
		  TIMER1_IR  = 0xff;          //reset all interrrupt flags
		  TIMER1_MCR = 0x04;          //stop timer on match
		  TIMER1_TCR = 0x01;          //start timer

		  //wait until delay time has elapsed
		  while (TIMER1_TCR & 0x01);
}
