//Includes the standard definitions any solitaire c file may need.
#include <stdio.h>
#define MAX_CARDS 52

#ifndef DEF_H
#define DEF_H

typedef struct t_card {
	char covered;
	char suit;
	int val;
} t_card;

typedef struct waste {
	t_card* stored;
	int size;
} waste;

typedef struct foundations {
	t_card* stored;
	int size;
} foundations;

typedef struct stock {
	t_card* stored;
	int size;
} stock;

typedef struct column {
	t_card* stored;
	int size;
} column;

typedef struct tableau {
	column* cols;
} tableau;

char is_opposite(t_card c1, t_card c2);
char is_valid_move(t_card c1, t_card c2);

t_card rmv_card(t_card* arr, int* size, int idx);
int getCardVal(char orig_val);

void set_limit(short l);
short get_limit();

void set_turn(short t);
short get_turn();

char val_to_char(char val);
int suit_pos(char c);

extern const int MAX_STR_SIZE;

extern short lim;
extern short t;

extern char sw_s;
extern char sw_f;
extern char sw_1;
extern char sw_3;
extern char sw_L;

#endif 