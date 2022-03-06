/*
 * game.h
 *
 *  Created on: April 14, 2020
 *      Author: Cameron Isbell
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"

#ifndef GAME_H
#define GAME_H

void handleDuplicate(t_card* duplicates, t_card* allCards, t_card toCheck, int* numDupes, int* numCards);
char contains(t_card toCheck, t_card* list, int numItems);
void setCard(char* str, t_card* card, char covered);

void readFile(char* filename);
char** getWords(char* fullLine, int* words);

int suit_pos(char c);
char val_to_char(char val);

#endif