/*
 * winnable.c
 *
 *  Created on: April 1, 2020
 *      Author: Cameron Isbell
 */
//START DECLARATIONS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_CARDS 52
#define MAX_MOVES 14

const int MAX_STR_SIZE = 100;
const int CARD_LEN = 3;

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

int readFile(char* filename, int numMoves);
int verifyValue(char val);
int verifySuit(char suit);
char** getWords(char* fullLine, int* words);
char* check_switch(char* args[], int numArgs, char* sw_m, int* moves);
int getCardVal(char orig_val);
int suit_pos(char c);
void add_move(char* toAdd, /*char** list*/ int* len);
void handleDuplicate(t_card* duplicates, t_card* allCards, t_card toCheck, int* numDupes, int* numCards);
char contains(t_card toCheck, t_card* list, int numItems);
void setCard(char* str, t_card* card, char covered);
t_card rmv_card(t_card* arr, int* size, int idx);
char is_opposite(t_card c1, t_card c2);
char val_to_char(char val);
void process_moves(char* toAdd, waste* wst, stock* stk, tableau* tabl, foundations* fnd);
char is_valid_move(t_card c1, t_card c2);
void get_valid_moves(waste wst, stock stk, tableau tabl, foundations fnd);
void copy(char** arr1, char** arr2, int len);

char won = 0;
char sw_v = 0;

char** winList;
int winDepth = 0;

int limit = -1;

int turn = 0;

//TESTING
int maxMoves = 1000;

//END DECLARATIONS

int main(int argc, char* argv[])
{
	int numMoves = -1;
	char* filename;
	
	//argv[0] is the name of the program
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-m") && (i+1) < argc) {
			maxMoves = atoi(argv[i +1]);
			sw_v = 1;
		}
		else if (!strcmp(argv[i], "-v")) {
			sw_v = 1;
		}
		else if (argv[i][0] != '-' && argv[i -1][0] != '-') {
			filename = malloc((strlen(argv[i]) + 1) * sizeof(char));
			strcpy(filename, argv[i]);
		}
		
	}
	
	readFile(filename, numMoves);
}

int readFile(char* filename, int numMoves)
{
	
	FILE *f;

	f = fopen(filename, "r");

	//file is invalid if it does not exist
	if (!f) {
		f = stdin;
	}

	char* currentWord;
	char currentLine[MAX_STR_SIZE];

	int flipLine = 0;
	int flip = 0;
	limit = -1;
	int line = 0;
	int tracker = 0;
	int commandNext = 0;
	int numWords = 0;
	int startLine = 0;
	int numCards = 0;
	int numDuplicates = 0;
	int numCovered = 0;
	int cMoves = 1;

	t_card* allCards = malloc(sizeof(t_card) * MAX_CARDS);
	t_card* duplicates = malloc(sizeof(t_card) * MAX_CARDS);

	tableau tabl;
		tabl.cols = malloc(sizeof(column) * 7);
		for (int ib = 0; ib < 7; ib++) {
			tabl.cols[ib].stored = malloc(MAX_CARDS * sizeof(t_card));
		}

	stock stk;
		stk.size = 0;
		stk.stored = malloc(MAX_CARDS * sizeof(t_card));

	waste wst;
		wst.size = 0;
		wst.stored = malloc(MAX_CARDS * sizeof(t_card));

	foundations fnd;
		fnd.size = 0;
		fnd.stored = malloc(4 * sizeof(t_card));

	//Initialize columns of t to all have a size of 0
	//Makes it clear that nothing is currently stored in that column
	for (int ia = 0; ia < 7; ia++) {
		tabl.cols[ia].size = 0;
	}

	//runs until the file is completed, reads a line from the file at a time
	while (fgets(currentLine, MAX_STR_SIZE, f)) {

		//create a copy of the current line of the file and use it to split into an array delimited by spaces
		char tempLine[MAX_STR_SIZE];
		strcpy(tempLine, currentLine);

		char** wordList;
		wordList = getWords(tempLine, &numWords);

		if (numWords == 0) continue;

		for(int i = 0; i < numWords; i++) {
			currentWord = malloc(sizeof(char) * (strlen(wordList[i]) + 1));
			strcpy(currentWord, wordList[i]);
			if(currentWord[0] == '\0') continue;

			//strcmp returns 0 if the strings are equal, so result must be inverted
			if (!strcmp(currentWord,"RULES:")) {

				if (tracker != 0) {
					fprintf(stderr, "Incorrect Instruction: Expected RULES:, line %d\n", line);
					exit(0);
				}
				tracker = 1;
				continue;
			}
			else if (!strcmp(currentWord, "FOUNDATIONS:")){
				if (tracker != 1) {
					fprintf(stderr, "Incorrect Instruction: Expected FOUNDATIONS:, line %d", line);
					
					exit(0);
				}
				tracker = 2;
				continue;
			}

			else if (!strcmp(currentWord, "TABLEAU:")){
				if (tracker != 2) {
					fprintf(stderr, "Incorrect Instruction: Expected TABLEAU:, line %d", line);
					exit(0);
				}
				startLine = line + 1;
				tracker = 3;
				continue;
			}
			else if (!strcmp(currentWord, "STOCK:")){
				if (tracker != 3) {
					fprintf(stderr, "Incorrect Instruction: Expected STOCK:, line %d", line);
					exit(0);
				}
				flip = 0;
				tracker = 4;
				continue;
			}
			else if (!strcmp(currentWord, "MOVES:"))	{
				if(tracker != 4) {
					fprintf(stderr, "Incorrect Instruction: Expected MOVES:, line %d", line);
					exit(0);
				}
				tracker = 5;
				continue;
			}
			switch(tracker)
			{

				//If we enter here, then the file doesn't begin with RULES, making it invalid
				case 0:
					return 0;
				break;

				//RULES CASE
				case 1:

					if (!strcmp(currentWord, "turn")) {

						commandNext = 1;
					}
					else if(commandNext == 1) {

						//Turns the string into an int to be assigned
						turn = atoi(currentWord);
						commandNext = 0;
					}
					else if(!strcmp(currentWord, "unlimited")) {
						limit = -1;
					}
					else if (!strcmp(currentWord, "limit")) {

						commandNext = 2;
					}
					else if(commandNext == 2) {

						limit = atoi(currentWord);
						commandNext = 0;
					}

				break;

				//FOUNDATIONS CASE
				case 2:

					setCard(currentWord, &fnd.stored[suit_pos(currentWord[1])] , 0);

					//No point adding this card since it's essentially an empty spot
					if (currentWord[0] != '_') {
						handleDuplicate(duplicates, allCards, fnd.stored[fnd.size], &numDuplicates, &numCards);
					}
					else fnd.stored[suit_pos(currentWord[1])].val = 0; 
					fnd.size++;

				break;

				//TABLEAU CASE
				case 3:
					/*
						Covered / uncovered flip resets on each line, so if the line has changed, reset flipLine
						Each line is guaranteed to have a pipe, so we know that this will trigger whenever we go to the next line
						For this reason, we can use this to decrement column
					*/
					if (flip && flipLine != line) {
						flipLine = 0;
						flip = 0;
					}

					//Pipe sets flip to 1, switches from covered to uncovered
					if (!strcmp(currentWord, "|")) {
						flipLine = line;
						flip = 1;
					}
					//Covered case
					else {
						//Columns are seperated by line, so if we record the line it starts at we can determine which column we are in
						int currentCol = (startLine - line) + 6;
						int pos = tabl.cols[currentCol].size;
						if (currentCol < 0 || currentCol > 6) fprintf(stderr, "Too many columns at line %d", line);
						setCard(currentWord, &tabl.cols[currentCol].stored[pos], !flip);

						handleDuplicate(duplicates, allCards, tabl.cols[currentCol].stored[pos], &numDuplicates, &numCards);
						tabl.cols[currentCol].size++;
						numCovered += !flip;
					}

				break;

				//STOCK CASE
				case 4:
					//cards go in waste first, then stock after a pipe
					if (!strcmp(currentWord, "|")) {
						flip = 1;
					}
					//NOTE: last card in the waste is the top of the waste
					else if (!flip) {
						setCard(currentWord, &wst.stored[wst.size], 0);

						handleDuplicate(duplicates, allCards, wst.stored[wst.size], &numDuplicates, &numCards);
						wst.size++;
					}
					//NOTE: first card in the stock is the top of the stock
					else if (flip) {
						setCard(currentWord, &stk.stored[stk.size], 0);

						handleDuplicate(duplicates, allCards, stk.stored[stk.size], &numDuplicates, &numCards);
						stk.size++;
					}

				break;

				//MOVES CASE
				case 5:
					
					if (numMoves == -1 || cMoves - 1 < numMoves)
					{
						//turn over turn cards from stock to waste
						if (!strcmp(currentWord, "."))
						{
							int i = 0;
							while (i < turn)
							{
								if (stk.stored != 0) 
								{
									//put the first item in stk into the waste. As stk items are removed, they are shifted so we just get the first item again
									wst.stored[wst.size] = rmv_card(stk.stored, &stk.size, 0);
									wst.size++;
									i++;
								}
								else 
								{
									fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
									//print_state(tabl, wst, stk, fnd, turn, limit);
									exit(0);
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
									t_card top = wst.stored[wst.size - 1];
									int fndPos = suit_pos(top.suit);
									
									if (top.val - 1 == fnd.stored[fndPos].val)
									{
											//remove from waste, set value of foundation to be equal to top
										fnd.stored[fndPos] = rmv_card(wst.stored, &wst.size, wst.size - 1);
									}
									else {
										fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
										//print_state(tabl, wst, stk, fnd, turn, limit);
										exit(0);
									}
								}
								//tableau to foundation
								else
								{

									int startCol = (src - '0') - 1;
									int tablPos = tabl.cols[startCol].size -1;
									int fndPos = suit_pos(tabl.cols[startCol].stored[tablPos].suit);

									if (tabl.cols[startCol].stored[tablPos].covered != 1 &&
										(tabl.cols[startCol].stored[tablPos].val - 1  == fnd.stored[fndPos].val))
									{

										//                            Array of Cards              Size of that array     Index to remove at
										fnd.stored[fndPos] = rmv_card(tabl.cols[startCol].stored, &(tabl.cols[startCol].size), tablPos);
										if (tabl.cols[startCol].stored[tabl.cols[startCol].size -1].covered) {
									
											tabl.cols[startCol].stored[tabl.cols[startCol].size -1].covered = 0;
										}
									}
									else
									{
										fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
										//print_state(tabl, wst, stk, fnd, turn, limit);
										exit(0);
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
									int tablPos = tabl.cols[endCol].size -1;
									
									if (wst.size > 0) 
									{
										if (tablPos >= 0 && is_opposite(wst.stored[wst.size -1], tabl.cols[endCol].stored[tablPos]) &&
											(wst.stored[wst.size -1].val + 1 == tabl.cols[endCol].stored[tablPos].val))
										{
											tabl.cols[endCol].stored[tabl.cols[endCol].size] = rmv_card(wst.stored, &(wst.size), wst.size - 1);
											tabl.cols[endCol].size++;

										}
										else if (tabl.cols[endCol].size == 0 && wst.size > 0) 
										{
											tabl.cols[endCol].stored[0] = rmv_card(wst.stored, &(wst.size), wst.size - 1);
										}
										
										
										else
										{
											fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
											//print_state(tabl, wst, stk, fnd, turn, limit);
											exit(0);
										}
									}
									else 
									{
										fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
										//print_state(tabl, wst, stk, fnd, turn, limit);
										exit(0);
									}
									
									
								}
								//tableau to tableau
								else
								{
									int startCol = (src - '0') - 1;
									int endCol = (dest - '0') - 1;

									int sPos = tabl.cols[startCol].size -1;
									int ePos = tabl.cols[endCol].size -1;
									char found = 0;

									if (tabl.cols[endCol].size != 0) 
									{
										while (tabl.cols[startCol].stored[sPos].covered != 1 && !found && sPos >= 0)
										{
											// might be -1 
											if (tabl.cols[startCol].stored[sPos].val + 1 == tabl.cols[endCol].stored[ePos].val)
											{
												if (is_opposite(tabl.cols[startCol].stored[sPos], tabl.cols[endCol].stored[ePos]))
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
										while(tabl.cols[startCol].stored[sPos].covered != 1 && sPos >= 0) 
										{
											sPos--;
										}
										//When we reach one that is covered, the position we want to be at is the previous one
										sPos++;
										found = 1;
									}
										
									if (!found)
									{
										fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
										//print_state(tabl, wst, stk, fnd, turn, limit);
										exit(0);
									}
									
									//Takes advantage of the way rmv_card shifts cards over, does it until we are at the end of the column's stored array
									while (sPos != tabl.cols[startCol].size) 
									{
										ePos = tabl.cols[endCol].size;
										tabl.cols[endCol].stored[ePos] = rmv_card(tabl.cols[startCol].stored, &(tabl.cols[startCol].size), sPos);
										tabl.cols[endCol].size++;
									}
									if (tabl.cols[startCol].stored[tabl.cols[startCol].size -1].covered) {
									
										tabl.cols[startCol].stored[tabl.cols[startCol].size -1].covered = 0;
									}
									
									
								}
							}
						}	
						//reset stock
						else if (!strcmp(currentWord, "r"))
						{
							if (stk.size == 0 && limit != 0)
							{
								while (wst.size != 0)
								{
									stk.stored[wst.size -1] = wst.stored[wst.size -1];
									stk.size++;
									wst.size--;
									
								}
							}
							else {
								fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
								//print_state(tabl, wst, stk, fnd, turn, limit);
								exit(0);
							}
							
							
						}
				
						cMoves++;
					}
				//End of switch	
				break;
			}	
			free(currentWord);	
		}
			
		line++;	
	}
	
	
	get_valid_moves(wst, stk, tabl, fnd); 
	
	
	if (won) {
		for (int i = 0; i < winDepth; i++) {
			fprintf(stdout, "%s\n", winList[i]);
		}
	}
	else {
		fprintf(stdout, "Game is not winnable in %d moves\n", maxMoves);
	}
	
	
	
	
	
	
	fflush(stdout);


	//free(currentWord);
	free(stk.stored);
	free(wst.stored);
	free(fnd.stored);
	free(allCards);
	free(duplicates);

/*	for (int ic = 0; ic < 7; ic++) {
		free(tabl.cols[ic].stored);
	}
	free(tabl.cols);
*/
	fclose(f);
	return 1;
}

void get_valid_moves(waste wst, stock stk, tableau tabl, foundations fnd) {
	static int winRet = 1;
	static long numStates = 0;
	
	//Increment everytime we enter this function
	numStates++;
	
	/*
	
		Maximum of 14 Moves: 
			6 col -> col
			4 col -> fnd
			1 wst -> col
			1 wst -> fnd
			1 flip
			1 reset
	
		Maximum length of a move is 5 characters
	
	*/
	
	//14 items at 5 characters long
	char moveList[MAX_MOVES][5];
	//static char** winList;
	
	/*
		win condition:
			No covered cards
			1 or 0 waste
			Empty stock
	*/ 
	
	char failed = 0;
	for (int i = 0; i < 7; i++) {
		for (int j = tabl.cols[i].size - 1; j >= 0; j--) {
			if (tabl.cols[i].stored[j].covered) {
				failed = 1;
				i = 7; 
				break;
			}
		}
	}
	if (!failed) {
		if (stk.size == 0 && wst.size <= 1) {
			winList = malloc(winDepth * sizeof(char*));
			for (int i = 0; i < winDepth; i++) {
				winList[i] = malloc(5 * sizeof(char));
			}
			fprintf(stdout, "# Game is winnable in %d Moves\n", winDepth);
			won = 1;
			return;
		}
	}
	
	/*
	  POSSIBLE MOVES:
		
	  1) WST -> TAB
		*Opposite and 1 smaller
	  2) WST -> FND
		*Opposite and 1 smaller
	  3) TAB -> TAB
		*Iterate to position that is opp and 1 smaller
	  4) TAB -> FND
		*Opposite and 1 smaller
	  5) TURN OVER
		*Stock is not empty
	  6) RESET
		*Stock must be empty
	
		NOTE: **TOP** card of tableau is at index 0
	*/
	
	int numMoves = 0;
	
	
	
	//1)
	if (wst.size > 0) {
		for (int i = 0; i < 7; i++) {
			if (tabl.cols[i].size == 0) continue;
			//If the top of the waste is compatilble with the bottom card of the tableau
			t_card tablBot = tabl.cols[i].stored[tabl.cols[i].size - 1];
			
			if (is_valid_move(wst.stored[wst.size - 1], tablBot)) { 
				char toAdd[] = "w->";
				char dest = 1 + i + '0';
				strncat(toAdd, &dest, 1);
				strcpy(moveList[numMoves], toAdd);
				numMoves++;
			}
		}
	}
	//2) 
	if (is_valid_move(wst.stored[wst.size - 1], fnd.stored[suit_pos(wst.stored[wst.size - 1].suit)])) {
		strcpy(moveList[numMoves], "w->f");
		numMoves++;
	}
	
	//3) 
	//test each column against each other. 
	//source is i 
	for (int i = 0; i < 7; i++) { 
		
		//dest is j
		for (int j = 0; j < 7; j++) {
			//skip the part where src == dest
			if (j == i) { 
				continue;
			}
		
			int k = tabl.cols[i].size - 1;
			while (k >= 0 && !(tabl.cols[i].stored[k].covered)) {
				if ((is_valid_move(tabl.cols[i].stored[k], tabl.cols[j].stored[tabl.cols[j].size -1])) || 
					(tabl.cols[j].size == 0 && tabl.cols[i].stored[k].val == 13)) {
					
					char toAdd[5] = "";
					char src = 1+ i + '0';
					char dest = 1+ j + '0';
					strncat(toAdd, &src, 1);
					strcat(toAdd, "->");
					strncat(toAdd, &dest, 1);
					
					strcpy(moveList[numMoves], toAdd);
					numMoves++;
				}
				k--;
			}
		}
	}	
	
	//4) 
	for (int i = 0; i < 7; i++) {
		t_card current = tabl.cols[i].stored[tabl.cols[i].size -1];
		if (current.val == fnd.stored[suit_pos(current.suit)].val + 1) {
			char toAdd[5] = "";
			char src = 1 + i + '0';
			strncat(toAdd, &src, 1);
			strcat(toAdd, "->f");
			
			strcpy(moveList[numMoves], toAdd);
			numMoves++;
		}
	}
	
	//5) 
	if (stk.size != 0) {
		strcpy(moveList[numMoves], ".");
		numMoves++;
		
	}
	
	//6)
	if (stk.size == 0 && limit != 0) {
		strcpy(moveList[numMoves], "r\0");
		numMoves++;
	}

	//If we've exceed the amount of allotted moves
	//At winDepth == max, we already check for wins so we are out of moves
	if (winDepth >= maxMoves) {
		//This string of moves is not allowed, return back up
		winDepth--;
		return;
	}

	//Base case: no moves to perform
	if (!numMoves) {
		winDepth--;
		return;
	}
	
	int idex = 0;
	
	while (idex < numMoves && !won) {
		
		tableau tabl1;
		tabl1.cols = malloc(sizeof(column) * 7);
		for (int i = 0; i < 7; i++) {
			tabl1.cols[i].stored = malloc(MAX_CARDS * sizeof(t_card));
			tabl1.cols[i].size = tabl.cols[i].size;
		}

		stock stk1;
			stk1.size = stk.size;
			stk1.stored = malloc(MAX_CARDS * sizeof(t_card));

		waste wst1;
			wst1.size = wst.size;
			wst1.stored = malloc(MAX_CARDS * sizeof(t_card));

		foundations fnd1;
			fnd1.size = 4;
			fnd1.stored = malloc(4 * sizeof(t_card));
		
		for (int i = 0; i < 7; i++) {
			for(int j = 0; j < tabl.cols[i].size; j++) {
				
				tabl1.cols[i].stored[j].val = tabl.cols[i].stored[j].val;
				tabl1.cols[i].stored[j].suit = tabl.cols[i].stored[j].suit;
				tabl1.cols[i].stored[j].covered = tabl.cols[i].stored[j].covered;
			}	
		}
		
		for (int i = 0; i < wst.size; i++) {
			wst1.stored[i].val = wst.stored[i].val;
			wst1.stored[i].suit = wst.stored[i].suit;
			wst1.stored[i].covered = wst.stored[i].covered;
		
		}
		
		for (int i = 0; i < 4; i++) {
			fnd1.stored[i].val = fnd.stored[i].val;
			fnd1.stored[i].suit = fnd.stored[i].suit;
			fnd1.stored[i].covered = 0;
		}
		
		for (int i = 0; i < stk.size; i++) {
			stk1.stored[i].val = stk.stored[i].val;
			stk1.stored[i].suit = stk.stored[i].suit;
			stk1.stored[i].covered = stk.stored[i].covered;
		}
		
		//Increase the possible depth before going farther
		winDepth++;
		process_moves(moveList[idex], &wst1, &stk1, &tabl1, &fnd1);
		get_valid_moves(wst1, stk1, tabl1, fnd1);
		
		idex++;
	}	

	if (sw_v && !(numStates % 100000)) {
		fprintf(stdout, "%ld\n", numStates);
	}
	




	/* 
		When we find the winning sequence and return up, we will be in the loop.
		Break out of the loop, use the base case
		
		Add the move in the sequence to the list
	
	*/
	if (won) {
		//While returning back up, record the moves that led to a win state in this order.
		//Additionally, index would increment before exiting the loop, so subtract 1
		if (!winList) return;
		strcpy(winList[winDepth - winRet], moveList[idex - 1]);
		winRet++;
		return;
	}
	
	winDepth--;
	
	//Base case 3, kinda? Processed all moves, no dice, time to go back up
	return;
	
}

//repurposed from previous part
void process_moves(char* currentWord, waste* wst, stock* stk, tableau* tabl, foundations* fnd) {
	
	//turn over turn cards from stock to waste
	if (!strcmp(currentWord, ".\0"))
	{
		int i = 0;
		while (i < turn)
		{
			if ((*stk).stored != 0) 
			{
				//put the first item in (*stk) into the waste. As (*stk) items are removed, they are shifted so we just get the first item again
				(*wst).stored[(*wst).size] = rmv_card((*stk).stored, &(*stk).size, 0);
				(*wst).size++;
				i++;
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
				t_card top = (*wst).stored[(*wst).size - 1];
				int fndPos = suit_pos(top.suit);
				
				if (top.val - 1 == (*fnd).stored[fndPos].val)
				{
					//remove from waste, set value of foundation to be equal to top
					(*fnd).stored[fndPos] = rmv_card((*wst).stored, &(*wst).size, (*wst).size - 1);
				}
			}
			//tableau to foundation
			else
			{
				int startCol = (src - '0') - 1;
				int tablPos = (*tabl).cols[startCol].size -1;
				int fndPos = suit_pos((*tabl).cols[startCol].stored[tablPos].suit);
				
				if ((*tabl).cols[startCol].stored[tablPos].covered != 1 &&
					((*tabl).cols[startCol].stored[tablPos].val - 1  == (*fnd).stored[fndPos].val))
				{

					//                            Array of Cards              Size of that array     Index to remove at
					(*fnd).stored[fndPos] = rmv_card((*tabl).cols[startCol].stored, &((*tabl).cols[startCol].size), tablPos);
						
					if ((*tabl).cols[startCol].stored[(*tabl).cols[startCol].size -1].covered) {
						
						(*tabl).cols[startCol].stored[(*tabl).cols[startCol].size -1].covered = 0;
					}
				}		
			}
		}
		//dest != f
		else 	
		{
			//waste to tableau
			if (src == 'w')
			{
				int endCol = (dest - '0') - 1;
				int tablPos = (*tabl).cols[endCol].size -1;
				
				if ((*wst).size > 0) 
				{
					if (tablPos >= 0 && is_opposite((*wst).stored[(*wst).size -1], (*tabl).cols[endCol].stored[tablPos]) &&
						((*wst).stored[(*wst).size -1].val + 1 == (*tabl).cols[endCol].stored[tablPos].val))
					{
						(*tabl).cols[endCol].stored[(*tabl).cols[endCol].size] = rmv_card((*wst).stored, &((*wst).size), (*wst).size - 1);
						(*tabl).cols[endCol].size++;

					}
					else if ((*tabl).cols[endCol].size == 0 && (*wst).stored[(*wst).size - 1].val == 13) 
					{
						(*tabl).cols[endCol].stored[0] = rmv_card((*wst).stored, &((*wst).size), (*wst).size - 1);
					}
				}
			}
			//tableau to tableau
			else
			{
				int startCol = (src - '0') - 1;
				int endCol = (dest - '0') - 1;
				int sPos = (*tabl).cols[startCol].size -1;
				int ePos = (*tabl).cols[endCol].size -1;
				char found = 0;

				if ((*tabl).cols[endCol].size != 0) 
				{
					while ((*tabl).cols[startCol].stored[sPos].covered != 1 && !found && sPos >= 0)
					{
						// might be -1 
						if ((*tabl).cols[startCol].stored[sPos].val + 1 == (*tabl).cols[endCol].stored[ePos].val)
						{
							if (is_opposite((*tabl).cols[startCol].stored[sPos], (*tabl).cols[endCol].stored[ePos]))
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
					while((*tabl).cols[startCol].stored[sPos].covered != 1 && sPos >= 0) 
					{
						sPos--;
					}
					//When we reach one that is covered, the position we want to be at is the previous one
					sPos++;
					found = 1;
				}
			
				//Takes advantage of the way rmv_card shifts cards over, does it until we are at the end of the column's stored array
				while (sPos != (*tabl).cols[startCol].size) 
				{
					ePos = (*tabl).cols[endCol].size;
					(*tabl).cols[endCol].stored[ePos] = rmv_card((*tabl).cols[startCol].stored, &((*tabl).cols[startCol].size), sPos);
					(*tabl).cols[endCol].size++;
				}
				
				if ((*tabl).cols[startCol].stored[(*tabl).cols[startCol].size -1].covered) {
				
					(*tabl).cols[startCol].stored[(*tabl).cols[startCol].size -1].covered = 0;
				}			
			}
		}
	}
	else if (!strcmp(currentWord, "r"))
	{
		if ((*stk).size == 0)
		{
			while ((*wst).size != 0)
			{
				(*stk).stored[(*wst).size -1] = (*wst).stored[(*wst).size -1];
				(*stk).size++;
				(*wst).size--;
			}
		}
	
		limit--;
	}
}

/*
	c1 is source, c2 is destination
	A move is valid if the cards are opposite and the src is 1 larger than the dest
		and if neither card is covered
*/
char is_valid_move(t_card c1, t_card c2) {
	if (is_opposite(c1, c2) && (c1.val == (c2.val - 1)) && !c1.covered && !c2.covered) {
		return 1;
	}
	return 0;
}

char val_to_char(char val) {
	if (val > 9) {
		if (val == 13) return 'K';
		if (val == 12) return 'Q';
		if (val == 11) return 'J';
		if (val == 10) return 'T';
	}
	else {
		return val + '0';
	}
	
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


//Returns the position the card should be at in foundation
int suit_pos(char c) {
	if (c == 'h') return 0;
	else if (c == 'd') return 1;
	else if (c == 'c') return 2;
	else if (c == 's') return 3;
}

void setCard(char* str, t_card* card, char covered) {
	(*card).suit = str[1];
	if (str[0] != '_') {
		(*card).val = getCardVal(str[0]);
	}
	else (*card).val = str[0];
	(*card).covered = covered;
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

/*
* Handles duplicates by checking if the card is already listed in allCards
* Will always add a duplicate to the list of all cards so it checks for multiple
* duplicates, and because the output will be invalid anyway
*/
void handleDuplicate(t_card* duplicates, t_card* allCards, t_card toCheck, int* numDupes, int* numCards)
{
	if (contains(toCheck, allCards, *numCards)) {
		duplicates[*numDupes] = toCheck;
		(*numDupes)++;
	}
	allCards[(*numCards)] = toCheck;
	(*numCards)++;
}

//adasd

/*
 * If val is in the list of strings, returns the index.
 * Otherwise, return 0
 */
char contains(t_card toCheck, t_card* list, int numItems)
{
	int i = 0;
	for (i = 0; i < numItems; i++) {
		if (toCheck.val == list[i].val && toCheck.suit == list[i].suit) {
			return 1;
		}
	}
	return 0;
}

/*
 * Checks if the suit of the given card is valid
 */
int verifySuit(char suit)
{
	if (suit == 'c' || suit == 'd' || suit == 'h' || suit == 's') return 1;
	return 0;
}

/*
 * Checks if the value of the card is valid
 */
int verifyValue(char val) {
	if (val == 'K' || val == 'Q' || val == 'J' || val == 'T' || val == 'A' ||  (val <= '9' && val >= '2' )) {
		return 1;
	}
	return 0;
}


/*
 * Takes a string and splits it up into an array of strings, delimited by spaces
 * POTENTIAL ISSUE: Comments not separated from commands by spaces
 */

char** getWords(char* fullLine, int* words)
{
	int i = 0;
	int numWords = 0;
	int maxWordLen = 0;
	char* delimiter = " ";

	char* lineCopy1 = malloc(strlen(fullLine) * sizeof(char));
	strcpy(lineCopy1, fullLine);
	char* current;

	current = strtok(fullLine, delimiter);

	//stop counting words when we either run out of words or reach a comment
	while (current != NULL && current[0] != '#') {
		if (strlen(current) > maxWordLen) maxWordLen = strlen(current);
		current = strtok(NULL, delimiter);
		numWords++;
	}

	char** wordList = malloc(numWords * sizeof(char*));

	current = strtok(lineCopy1, delimiter);

	//Now that we know the number of words, we are able to put them in the return array
	while (current != NULL && current[0] != '#') {

		wordList[i] = malloc(maxWordLen * sizeof(char));

		//Make any newlines at the end of a string into the end of the string
		if(current[strlen(current) - 1] == '\n') current[strlen(current) - 1] = '\0';
		
		if (current[strlen(current) - 1] == '\r') current[strlen(current) - 1] = '\0';
		
		strcpy(wordList[i], current);
		current = strtok(NULL, delimiter);
		i++;
	}

	*words = numWords;
	return wordList;
}

