
#ifndef TIME_INTERVAL_H_
#define TIME_INTERVAL_H_

#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include "snake.h"

extern volatile tS32 timeLeft;

void wait(int delay);

void timeProcStart(void* arg);
void start();
void stop();
void reset();
void disableLed(int ledNumber);
void enableAllLeds();


#endif /* TIME_INTERVAL_H_ */
