#include "play.h"
#include <termbox.h>

float designedWidth = 80;
float designedHeight = 25;

char deck[52][3] = {
	"Ac", "2c", "3c", "4c", "5c", "6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc",
	"Ad", "2d", "3d", "4d", "5d", "6d", "7d", "8d", "9d", "Td", "Jd", "Qd", "Kd",
	"Ah", "2h", "3h", "4h", "5h", "6h", "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh",
	"As", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s", "Ts", "Js", "Qs", "Ks"
};

void shuffle(char en, unsigned int seed) {
	if (en) srand(seed);
	else srand(time(0));
	
	for (int i = 0; i < 52; i++) {
		int j = rand() % 52;
		
		if (i != j) swap(deck[i], deck[j]);
	}
}

void swap (char c1[], char c2[]) {
	char temp[strlen(c1) + 1];
	strcpy(temp, c1);
	strcpy(c1, c2);
	strcpy(c2, temp);
}

state make_game(char en, unsigned int seed) {
	shuffle(en,seed);
	state st;
	
	st.tabl.cols = malloc(sizeof(column) * 7);
	for (int i = 0; i < 7; i++) {
		st.tabl.cols[i].stored = malloc(MAX_CARDS * sizeof(t_card));
		st.tabl.cols[i].size = 0;
	}

	st.stk.size = 0;
	st.stk.stored = malloc(MAX_CARDS * sizeof(t_card));

	st.wst.size = 0;
	st.wst.stored = malloc(MAX_CARDS * sizeof(t_card));

	st.fnd.size = 0;
	st.fnd.stored = malloc(4 * sizeof(t_card));
	
	for (int i = 0; i < 4; i++) {
		st.fnd.stored[i].val = 0;
	}
	st.fnd.stored[0].suit = 'h';
	st.fnd.stored[1].suit = 'd';
	st.fnd.stored[2].suit = 'c';
	st.fnd.stored[3].suit = 's';
	
	
	//filling random cards into tableau right to left
	int fill = 7;
	
	short r = 51;
	for (int i = 6; i >= 0; i--) {
		
		for (int j = 0; j < fill - 1; j++) {
			
			set_card(deck[r], &st.tabl.cols[i].stored[j], 1);
			st.tabl.cols[i].size++;
			//swap the item at the end of the deck with the card to be removed
			
			r--;
		}
		
		set_card(deck[r], &st.tabl.cols[i].stored[fill - 1], 0);
		
		r--;
		st.tabl.cols[i].size++;
		fill--;
	}
	
	//filling all remaining cards into the stock	
	int i = 0;
	while (r >= 0) {
		set_card(deck[r], &st.stk.stored[i], 0);
		st.stk.size++;
		i++;
		r--;
	}
	
	return st;
}

//random start
void r_start(unsigned int seed) { 
	//defaults to unlimited
	if (!sw_L) set_limit(-1);
	if(!sw_3 && !sw_1) set_turn(1);
	
	
	state st = make_game(sw_s, seed);
	play_game(st);
}

//file start
void f_start(tableau tabl, foundations fnd, stock stk, waste wst) {
	state st;
	st.tabl = tabl;
	st.fnd = fnd;
	st.stk = stk;
	st.wst = wst;
	
	play_game(st);
	
}

void play_game(state st) { 
	state prv;
	
	tb_init();
	display_game(st);
	
	struct tb_event e;

	while (e.ch != 'q') {
		tb_poll_event(&e);
		
		if (e.ch) {
			
			if (e.ch >= '1' && e.ch <= '7') {
				char mv[5] = "";
				
				char src =  e.ch;
				
				strncat(mv, &src, 1);
				strcat(mv, "->");
				
				//reset before polling for more information
				e.ch = 0;
				while (!(e.ch >= '1' && e.ch <= '7') && e.ch != 'f' ) {
					tb_poll_event(&e);
				}
				char dest;
				if (e.ch != 'f') dest = e.ch;
				else dest = 'f';
				
				strncat(mv, &dest, 1);
				prv = st;
				make_move(&st, mv);
				display_game(st);
				continue;
			}
			else if (e.ch == 'w') {
				char mv[5] = "w->";
				
				e.ch = 0;
				while (!(e.ch >= '1' && e.ch <= '7') && e.ch != 'f') {
					tb_poll_event(&e);
				}
				char dest;
				if (e.ch != 'f') dest = e.ch;
				else dest = 'f';
				
				strncat(mv, &dest, 1);
				prv = st;
				make_move(&st, mv);
				display_game(st);
				continue;
				
			}
			else if (e.ch == '.') {
			
				make_move(&st, ".");
				display_game(st);
				continue;
			}
			else if (e.ch == 'r') {
				prv = st;
				make_move(&st, "r");
				display_game(st);
				continue;
			}
			
			/*else if (e.ch == 'u') {
				st.wst = prv.wst;
				st.fnd = prv.fnd;
				st.tabl = prv.tabl;
				st.stk = prv.stk;
		
				display_game(st);
				st = prv;
				
				continue;
			}*/
			else continue;
			
			
			display_game(st);
		}
		e.ch = 0;
	}
	
	tb_shutdown();
}




void display_game(state st) { 
	tb_clear();
	
	struct tb_cell spade;
	struct tb_cell club;
	
	for (int i = 0; i < tb_width(); i++) {
		struct tb_cell w;
		w.fg = TB_CYAN;
		w.bg = TB_DEFAULT;
		w.ch = '-';
		
		tb_put_cell(i, tb_height() - 4, &w);
	}
	
	char inst1[] = "'q' : quit";
	char inst2[] = "'.' : next card";
	char inst3[] = "'r' : reset stock";
	char inst4[] = "'x' 'y' : move from position x to position y";
	char inst5[] = "some # = col, f = foundation, w = waste";
	
	send_def(0, tb_height() - 3, inst1);
	send_def(0, tb_height() - 2, inst2);
	
	send_def(strlen(inst2) + 2, tb_height() - 2, inst3);
	send_def(strlen(inst2) + strlen(inst3) + 4, tb_height() - 2, inst4);
	
	send_def(0, tb_height() -1, inst5);

	
	float xMax = (tb_width() / designedWidth) * 3  + 1;
	float yMax = (tb_height() / designedHeight) * 4 + 1;
	int tWidth = (7 * (1 + xMax));
	
	float xStart = tb_width() - xMax - (strlen("Foundations") /2);
	
	draw_foundation(st.fnd.stored, tWidth + 10, tb_height() / 2, tWidth + 12 + xMax, (yMax  + (tb_height() / 2) - 2));
	
	draw_stock(st.stk, tWidth + 12, tb_height() / 8, tWidth + 12 + xMax, yMax + (tb_height()/ 8));

	draw_waste(st.wst, tWidth + 20, tb_height() / 8, tWidth + 20 + xMax, (yMax + tb_height() / 8) - 2);
	//width of the screen - width a tableau takes up
	
	float yStart = 3;
	
	xMax++;
	yMax++;
	
	send_def(tWidth /2 , 0, "Tableau"); 
	
	draw_tableau(st.tabl, 0, yStart, xMax, yMax, 1); 
	
	tb_present();
}

void draw_tableau(tableau tabl, int x1, int y1, int x2, int y2, int space) {
	
	struct tb_cell blank;
	blank.fg = TB_WHITE;
	blank.bg = TB_WHITE;

	int diff = (x2 - x1);
	int count = 0;
	int offset = 0;

	for (int i = 0; i < 7; i++) {
		struct tb_cell num;
		num.bg = TB_CYAN;
		num.fg = TB_BLACK;
		num.ch = '0' + i + 1;
		
		//space is the space between two cards, count is the number of columns, diff is the difference between one end of the card and another
		int p1 = x1 + (count * diff) + ((count + 1) * space);
		int p2 = x2 + count * (space + diff);
		
		tb_put_cell((p1 + p2)/2, y1 - 1, &num);
		
		for (int j = 0; j < tabl.cols[i].size; j++) {
			struct tb_cell su;
			struct tb_cell v; 
			
			su.bg = TB_WHITE;
			v.bg = TB_WHITE;
			v.fg = TB_BLACK;
			v.ch = val_to_char(tabl.cols[i].stored[j].val);
			
			if ((tabl.cols[i].stored[j].suit == 'h') || (tabl.cols[i].stored[j].suit == 'd')) {
				su.fg = TB_RED;
				if (tabl.cols[i].stored[j].suit == 'h') su.ch = 0x02665;
				else su.ch = 0x025C6;
			}
			else {
				su.fg = TB_BLACK;
				if (tabl.cols[i].stored[j].suit == 'c') su.ch = 0x02663;
				else su.ch = 0x02660;	
			}
			
			if (tabl.cols[i].stored[j].covered) {
				struct tb_cell cover;
				cover.bg = TB_DEFAULT;
				cover.fg = TB_MAGENTA;
				cover.ch = '#';

				for (int x = p1; x < p2; x++) {	
					tb_put_cell(x, y1 + j, &cover);
				}
			}
			else {
				draw_card(tabl.cols[i].stored[j].val, tabl.cols[i].stored[j].suit, p1, y1 + j, p2 , y2 + j);
			}
		}
		count++;
	}
}

void draw_foundation(t_card* fnd, int x1, int y1, int x2, int y2) {

	int start = x1;
	int end;
	for (int i = 0; i < 4; i++) {
		
		int p1 = x1 + (i * (x2 - x1)) + ((i + 1) * 2);
		int p2 = x2 + i * (2 + (x2 - x1));
		draw_card(fnd[i].val, fnd[i].suit, p1, y1, p2, y2); 
		end = p2;

	
	}
	
	
	send_def((end + start) /2 - 4, y1 - 2, "Foundations");
	
}

void draw_stock (stock stk, int x1, int y1, int x2, int y2) {
	if (stk.size == 0) return;
	
	struct tb_cell c;
	c.fg = TB_MAGENTA;
	c.bg = TB_GREEN;
	c.ch = '#';
	
	for (int y = y1; y < y2; y++) {
		for (int x = x1; x < x2; x++) {
			tb_put_cell(x, y, &c);
		}
	}
	
	//max number would be 52.
	char left[3];
	sprintf(left, "%d", stk.size);
	
	c.fg = TB_WHITE;
	c.bg = TB_DEFAULT;
	
	for (int i = 0; i < strlen(left); i++) {
		c.ch = left[i];
		tb_put_cell(x1 + i + 1, y2, &c);
	}
}

void draw_waste(waste wst, int x1, int y1, int x2, int y2) {

	send_def((x1 + x2) /2 - 6, y1 - 3, "Waste");

	
	if (wst.size == 0) return;
	struct tb_cell s;
	s.bg = TB_WHITE;
	
	struct tb_cell v;
	v.bg = TB_WHITE;
	v.fg = TB_BLACK;
	
	struct tb_cell b;
	b.fg = TB_WHITE;
	b.bg = TB_WHITE;
	
	
	int k = 0;
	int i;
	if (wst.size < 3) i = 0;
	else i = wst.size - 3;
	
	while (i < wst.size) {
		
		if (wst.stored[i].suit == 'h' || wst.stored[i].suit == 'd') {
			s.fg = TB_RED;
			if (wst.stored[i].suit == 'h') s.ch = 0x02665;
			else s.ch = 0x025C6;
		}
		else {
			s.fg = TB_BLACK;
			if (wst.stored[i].suit == 'c') s.ch =  0x02663;
			else s.ch = 0x02660;	
		}
		
		v.ch = val_to_char(wst.stored[i].val);
		
		if (i == wst.size -1) break;
		
		
		tb_put_cell(x1, y1 + k, &s);
		tb_put_cell(x1 + 1, y1 + k, &v);
	
		for (int j = x1 + 2; j < x2; j++) {
			tb_put_cell(j, y1 + k, &b);
		}
		
		i++;
		k++;
	}
	
	
	if (wst.size != 0) {
		for (int y = y1 + k; y <= y2 + k; y++) {
			for (int x = x1; x < x2; x++) {
				tb_put_cell(x, y, &b);
			}
		}	
		
		tb_put_cell(x1, y1 + k, &s);
		tb_put_cell(x2 -1 , y2 + k, &s);
		
		tb_put_cell(x1+1, y1 + k, &v);
		tb_put_cell(x2-2, y2 + k, &v);
	}
	

}

void draw_card(char val, char suit, int x1, int y1, int x2, int y2) {
	struct tb_cell c;
	c.fg = TB_WHITE;
	c.bg = TB_WHITE;
	
	for (int y = y1; y <= y2; y++) {
		for (int x = x1; x < x2; x++) {
			tb_put_cell(x, y, &c);
		}
	}
	struct tb_cell su;
	su.bg = TB_WHITE;
	
	if (suit == 'h' || suit == 'd') {
		su.fg = TB_RED;
		if (suit == 'h') su.ch = 0x02665;
		else su.ch = 0x025C6;
	}
	else {
		su.fg = TB_BLACK;
		if (suit == 'c') su.ch =  0x02663;
		else su.ch = 0x02660;	
	}

	tb_put_cell(x1, y1, &su);
	tb_put_cell(x2 -1 , y2, &su);
	
	if (val == 0) {
		su.ch = 'X';
		su.fg = TB_RED;
	}
	else {
		su.fg = TB_BLACK;
		su.ch = val_to_char(val);
	}
	tb_put_cell(x1+1, y1, &su);
	tb_put_cell(x2-2, y2, &su);
}

/*
	Sends the characters that define an event.
	All should be the same color scheme for consistency
*/
void send_def(int x, int y, char given[]) { 
	struct tb_cell c;
	c.fg = TB_BLACK;
	c.bg = TB_CYAN;

	for (int i = 0; i < strlen(given); i++) {
		c.ch = given[i];
		tb_put_cell(x, y, &c);
		x++;
	}

}

void set_card(char* str, t_card* card, char covered) {
	(*card).suit = str[1];
	(*card).val = getCardVal(str[0]);
	(*card).covered = covered;
}


void make_move(state* st, char currentWord[]) { 

	//turn over turn cards from stock to waste
	if (!strcmp(currentWord, "."))
	{
		int i = 0;
		
		while (i < get_turn())
		{
			if ((*st).stk.size > 0) 
			{
				//put the first item in stk into the waste. As stk items are removed, they are shifted so we just get the first item again
				(*st).wst.stored[(*st).wst.size] = rmv_card((*st).stk.stored, &(*st).stk.size, 0);
				(*st).wst.size++;
				i++;
			}
			else 
			{
				return;
			}
		}
	}

	//If we're at an arrow instruction
	else if (strlen(currentWord) == 4 && (currentWord[1] == '-' && currentWord[2] == '>'))
	{
		char src = currentWord[0];
		char dest = currentWord[3];
		//Traverse the source until we find a valid move onto dest. If no such move is given, move is invalid.

		if (dest == 'f')
		{
			//waste to foundation
			if(src == 'w')
			{
				t_card top = (*st).wst.stored[(*st).wst.size - 1];
				int fndPos = suit_pos(top.suit);
					
				if (top.val - 1 == (*st).fnd.stored[fndPos].val)
				{
					//remove from waste, set value of foundation to be equal to top
					(*st).fnd.stored[fndPos] = rmv_card((*st).wst.stored, &((*st).wst.size), (*st).wst.size - 1);
				}
				else {
					return;
				}
			}
			//tableau to foundation
			else
			{
				int startCol = (src - '0') - 1;
				int tablPos = (*st).tabl.cols[startCol].size -1;
				int fndPos = suit_pos((*st).tabl.cols[startCol].stored[tablPos].suit);

				if ((*st).tabl.cols[startCol].stored[tablPos].covered != 1 &&
					((*st).tabl.cols[startCol].stored[tablPos].val - 1  == (*st).fnd.stored[fndPos].val))
				{

				    //                            Array of Cards              Size of that array     Index to remove at
					(*st).fnd.stored[fndPos] = rmv_card((*st).tabl.cols[startCol].stored, &((*st).tabl.cols[startCol].size), tablPos);
					if ((*st).tabl.cols[startCol].stored[(*st).tabl.cols[startCol].size -1].covered) {
									
						(*st).tabl.cols[startCol].stored[(*st).tabl.cols[startCol].size -1].covered = 0;
					}
				}
				else
				{
					return;
				}
			}
		}
		//dest != f
		else 	
		{
			//waste to tableau
			if (src == 'w')
			{
				int endCol = (dest - '0') -1;
				int tablPos = (*st).tabl.cols[endCol].size -1;
				
				if ((*st).wst.size > 0) 
				{
					if (tablPos >= 0 && is_opposite((*st).wst.stored[(*st).wst.size -1], (*st).tabl.cols[endCol].stored[tablPos]) &&
						((*st).wst.stored[(*st).wst.size -1].val + 1 == (*st).tabl.cols[endCol].stored[tablPos].val))
					{
						(*st).tabl.cols[endCol].stored[(*st).tabl.cols[endCol].size] = rmv_card((*st).wst.stored, &((*st).wst.size), (*st).wst.size - 1);
						(*st).tabl.cols[endCol].size++;

					}
					else if ((*st).tabl.cols[endCol].size == 0 && (*st).wst.size > 0) 
					{
						if ((*st).wst.stored[(*st).wst.size].val == 13) {
							(*st).tabl.cols[endCol].stored[0] = rmv_card((*st).wst.stored, &((*st).wst.size), (*st).wst.size - 1);
						}
						else return;
					}
					else
					{
						return;
					}
				}
				else 
				{
					return;
				}
			}
			//tableau to tableau
			else
			{
				int startCol = (src - '0') - 1;
				int endCol = (dest - '0') - 1;

				int sPos = (*st).tabl.cols[startCol].size -1;
				int ePos = (*st).tabl.cols[endCol].size -1;
				char found = 0;

				if ((*st).tabl.cols[endCol].size != 0) 
				{
					while ((*st).tabl.cols[startCol].stored[sPos].covered != 1 && !found && sPos >= 0)
					{
						// might be -1 
						if ((*st).tabl.cols[startCol].stored[sPos].val + 1 == (*st).tabl.cols[endCol].stored[ePos].val)
						{
							if (is_opposite((*st).tabl.cols[startCol].stored[sPos], (*st).tabl.cols[endCol].stored[ePos]))
							{
								found = 1;
								break;
							}
							else break;
						}
						sPos--;
					}
				}
				else 
				{
					while((*st).tabl.cols[startCol].stored[sPos].covered != 1 && sPos >= 0) 
					{
						sPos--;
					}
					//When we reach one that is covered, the position we want to be at is the previous one
					sPos++;
					if ((*st).tabl.cols[startCol].stored[sPos].val == 13) found = 1;
				}
				
				if (!found)
				{
					return;
				}
				
				//Takes advantage of the way rmv_card shifts cards over, does it until we are at the end of the column's stored array
				while (sPos != (*st).tabl.cols[startCol].size) 
				{
					ePos = (*st).tabl.cols[endCol].size;
					(*st).tabl.cols[endCol].stored[ePos] = rmv_card((*st).tabl.cols[startCol].stored, &((*st).tabl.cols[startCol].size), sPos);
					(*st).tabl.cols[endCol].size++;
				}
				if ((*st).tabl.cols[startCol].stored[(*st).tabl.cols[startCol].size -1].covered) {
				
					(*st).tabl.cols[startCol].stored[(*st).tabl.cols[startCol].size -1].covered = 0;
				}
			}
		}
	}	
	//reset stock
	else if (!strcmp(currentWord, "r"))
	{
		struct tb_cell test;
		test.fg = TB_WHITE;
		test.bg = TB_BLACK;
		test.ch = get_limit() + '0';
		tb_put_cell(1,1, &test);
	
		if ((*st).stk.size == 0 && get_limit() != 0)
		{
			while ((*st).wst.size != 0)
			{
				(*st).stk.stored[(*st).wst.size -1] = (*st).wst.stored[(*st).wst.size -1];
				(*st).stk.size++;
				(*st).wst.size--;
			}
			set_limit(get_limit() - 1);
		}
		else {
			return;
		}
	}
}