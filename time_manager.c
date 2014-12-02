#include "time_manager.h"
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
	printf("time proc start\n");
	start();
	for(;;){
		if(timeLeft > 0){
			if(timeLeft % 1000 == 0){
				printf("\n waiting sec, time left %d\n", timeLeft / 1000);
			}
			if(timeLeft == 0){
				stop();
			}

			wait(1);
			timeLeft = timeLeft - 1;

			if(((timeLeft - 1) % (begin_time / 8)) == 0){
				disableLed((timeLeft - 1) / (begin_time / 8));
			}
		}
	}
}

void disableLed(tS32 ledNumber){
	//printf("disabling led %d\n", ledNumber);
	setPca9532Pin(ledNumber + 8, 1);
}
void enableAllLeds(){
	//printf("Enabling all leds\n");
	int i;
	for(i = 0; i < 7; i++){
		setPca9532Pin(i + 8, 0);
	}
}

void start(){
	//printf("start called\n");
	timeLeft = begin_time;
	enableAllLeds();
}
void stop(){
	printf("game over\n");
	timeLeft = -1;
	setToLoseLife();
	reset();

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
