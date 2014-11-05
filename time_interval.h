
#ifndef TIME_INTERVAL_H_
#define TIME_INTERVAL_H_

#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"

extern volatile tU32 ms;

void wait(int delay);


#endif /* TIME_INTERVAL_H_ */
