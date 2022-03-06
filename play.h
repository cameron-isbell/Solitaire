#include <string.h>
#include <time.h>
#include "def.h"
#include <stdlib.h>

#ifndef PLAY_H
#define PLAY_H 

//A game state to be played and displayed
typedef struct state { 
	foundations fnd;
	waste wst;
	stock stk;
	tableau tabl;
	
} state;


void shuffle(char en, unsigned seed);
void swap(char c1[], char c2[]);

void set_card(char* str, t_card* card, char covered);
state make_game(char en, unsigned int seed);

void display_game(state st);
void make_move(state* st, char currentWord[]);

void play_game(state st);
void start();

void send_def (int x, int y, char given[]);
void draw_card(char val, char suit, int x1, int y1, int x2, int y2);

void draw_foundation(t_card* fnd, int x1, int y1, int x2, int y2);
void draw_tableau(tableau tabl, int x1, int y1, int x2, int y2, int space);

void draw_stock (stock stk, int x1, int y1, int x2, int y2); 
void draw_waste(waste wst, int x1, int y1, int x2, int y2);

void r_start();
void f_start(tableau tabl, foundations fnd, stock stk, waste wst);

extern char deck[52][3]; 

#endif