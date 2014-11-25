#ifndef EEPROM_UTIL_H_
#define EEPROM_UTIL_H_

#include "pre_emptive_os/api/general.h"
#include "snake.h"

void saveToEeprom(tU8 *data);
tU8* readFromEprom();

void saveScores(scores_t* scores);
scores_t* loadScores();




#endif /* EEPROM_UTIL_H_ */
