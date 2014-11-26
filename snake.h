/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    snake.h
 *
 * Description:
 *    Expose public functions related to the snake game.
 *
 *****************************************************************************/
#ifndef _SNAKE_H_
#define _SNAKE_H_

void playSnake(void);


typedef struct scores {
	tU8 nick1[3];
	tS16 score1;
	tU8 nick2[3];
	tS16 score2;
	tU8 nick3[3];
	tS16 score3;
	tU8 nick4[3];
	tS16 score4;

} scores_t;

#endif
