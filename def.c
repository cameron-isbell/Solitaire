//Implements the default functions any source file may need
#include "def.h"

short lim = -1;
short t = 0;

char is_opposite(t_card c1, t_card c2) {
	if (c1.suit == 'h' || c1.suit == 'd') {
		if (c2.suit == 'c' || c2.suit == 's') return 1;
		return 0;
	}
	else {
		if (c2.suit == 'h' || c2.suit == 'd') return 1;
		return 0;
	}
}

char is_valid_move(t_card c1, t_card c2) {
	if (is_opposite(c1, c2) && (c1.val == (c2.val - 1)) && !c1.covered && !c2.covered) {
		return 1;
	}
	return 0;
}

t_card rmv_card(t_card* arr, int* size, int idx) {
	int i = idx;
	t_card toReturn;
	toReturn.suit = arr[i].suit;
	toReturn.val = arr[i].val;
	toReturn.covered = arr[i].covered;

	//while there is a next item in the array, copy it into the current position
	while (i + 1 < *size) {
		arr[i].suit = arr[i+1].suit;
		arr[i].val = arr[i+1].val;
		arr[i].covered = arr[i+1].covered;

		i++;
	}
	
	//remove the last card
	arr[i].suit = 0;
	arr[i].val = 0;
	arr[i].covered = 0;

	(*size)--;

	return toReturn;
}

int getCardVal(char orig_val) {
	if (orig_val <= '9' && orig_val >= '2' ) {
		return orig_val - '0';
	}
	else if (orig_val == 'A') {
		return 1;
	}
	else if (orig_val == 'T') {
		return 10;
	}
	else if (orig_val == 'J') {
		return 11;
	}
	else if (orig_val == 'Q') {
		return 12;
	}
	else if (orig_val == 'K') {
		return 13;
	}
}

char val_to_char(char val) {
	if (val > 9) {
		if (val == 13) return 'K';
		if (val == 12) return 'Q';
		if (val == 11) return 'J';
		if (val == 10) return 'T';
	}
	else if (val == 1) return 'A';
	else {
		return val + '0';
	}	
}

int suit_pos(char c) {
	if (c == 'h') return 0;
	else if (c == 'd') return 1;
	else if (c == 'c') return 2;
	else if (c == 's') return 3;
}

short get_limit() { 
	return lim;
}

void set_limit(short l) { 
	lim = l;
}

short get_turn() {
	return t;
}

void set_turn(short turn) {
	t = turn;
}