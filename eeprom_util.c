#include "eeprom_util.h"


#include <lpc2xxx.h>
#include "startup/config.h"
#include <printf_P.h>
#include "eeprom.h"
#define MAX_LENGTH 13

static tU8 buffer[MAX_LENGTH];


void saveToEeprom(tU8 *data){
	tS8 errorCode;

	errorCode = eepromWrite(0x0000, data, sizeof(data));
	  if (errorCode == I2C_CODE_OK)
		  printf("Wrote successfully");
	  else
	  {
	    printf("\nT write string '%s' to address 0x0000", data);
	    printf("\n        - failed (error code = %d)!", errorCode);
	  }

}
tU8* readFromEprom(){
	tS8 errorCode;

	errorCode = eepromPageRead(0x0000, buffer, MAX_LENGTH);
	if (errorCode == I2C_CODE_OK)
	{
	    printf("Read successfully\n");
	}
	else
	{
		printf("Read unsuccesfully, code %d\n", errorCode);
	}

	return buffer;
}

void saveScores(scores_t* scores){

}
scores_t* loadScores(){
	return (void*)0;
}
