#include "eeprom_util.h"


#include <lpc2xxx.h>
#include "startup/config.h"
#include <printf_P.h>
#include "eeprom.h"
#define MAX_LENGTH 36

static tU8 buffer[MAX_LENGTH];
static scores_t m_scores;


void saveToEeprom(tU8 *data){
	tS8 errorCode;

	errorCode = eepromWrite(0x0000, data, sizeof(data) * 4);
	  if (errorCode == I2C_CODE_OK){
		  printf("Wrote successfully");
		  printf("\nT write string '%s' to address 0x0000", data);
	  }

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

static tU8 convertTotU8From(tS16 from){
	return (tS16) from;
}

static tS16 convertTotS16From(tU8 from){
	return (tS16)from;
}

void saveScores(scores_t* scores){
	printf("second score is %s %d \n", scores->nick2, scores->score2);

	buffer[0] = scores->nick1[0];
	buffer[1] = scores->nick1[1];
	buffer[2] = scores->nick1[2];
	buffer[3] = convertTotU8From(scores->score1);

	buffer[4] = scores->nick2[0];
	buffer[5] = scores->nick2[1];
	buffer[6] = scores->nick2[2];
	buffer[7] = convertTotU8From(scores->score2);

	buffer[8] = scores->nick3[0];
	buffer[9] = scores->nick3[1];
	buffer[10] = scores->nick3[2];
	buffer[11] = convertTotU8From(scores->score3);

	buffer[12] = scores->nick4[0];
	buffer[13] = scores->nick4[1];
	buffer[14] = scores->nick4[2];
	buffer[15] = convertTotU8From(scores->score4);

	saveToEeprom(buffer);
}
scores_t* loadScores(){
	tU8 *scores = readFromEprom();
	m_scores.nick1[0] = scores[0];
	m_scores.nick1[1] = scores[1];
	m_scores.nick1[2] = scores[2];
	m_scores.score1 = convertTotS16From(scores[3]);

	m_scores.nick2[0] = scores[4];
	m_scores.nick2[1] = scores[5];
	m_scores.nick2[2] = scores[6];
	m_scores.score2 = convertTotS16From(scores[7]);

	m_scores.nick3[0] = scores[8];
	m_scores.nick3[1] = scores[9];
	m_scores.nick3[2] = scores[10];
	m_scores.score3 = convertTotS16From(scores[11]);

	m_scores.nick4[0] = scores[12];
	m_scores.nick4[1] = scores[13];
	m_scores.nick4[2] = scores[14];
	m_scores.score4 = convertTotS16From(scores[15]);

	printf("first score is %s %d \n", m_scores.nick1, m_scores.score1);
	printf("second score is %s %d \n", m_scores.nick2, m_scores.score2);

	return &m_scores;
}
