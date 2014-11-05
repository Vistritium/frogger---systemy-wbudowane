#include "time_interval.h"
#include <printf_P.h>

static tU32 whenFinish;

/*
void set_timeout(tU32 timeout){

	VICV

	whenFinish = timeout + ms;
}
*/

static tS32 begin_time = 30 * 1000;

void timeProcStart(void* arg){
	start();
	for(;;){
		if(timeLeft > 0){
			wait(1);
			timeLeft = timeLeft - 1;

			if((timeLeft - 1) % (whenFinish / 8)){
				disableLed((timeLeft - 1) / (whenFinish / 8));
			}
		}
	}
}

void disableLed(tS32 ledNumber){
	printf("disabling led %d\n", ledNumber);
}
void enableAllLeds(){

}

void start(){
	timeLeft = begin_time;
}
void stop(){
	timeLeft = -1;
}
void reset(){
	start();
}



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
