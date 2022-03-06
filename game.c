#include "game.h"
#include "play.h"

//have to use extern to stop compiler complaints
const int MAX_STR_SIZE = 100;

char sw_s = 0;
char sw_f = 0;
char sw_1 = 0;
char sw_3 = 0;
char sw_L = 0;




int main(int argc, char* argv[])
{
	char* filename;
	unsigned int seed;
	
	
	//argv[0] is the name of the program
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-s") && (i+1) < argc) {
			seed = atoi(argv[i + 1]);
			sw_s = 1;
		}
		else if (!strcmp(argv[i], "-f") && (i+1) < argc) {
			filename = malloc((strlen(argv[i]) + 1) * sizeof(char));
			strcpy(filename, argv[i + 1]);
			sw_f = 1;
		}
		else if (!strcmp(argv[i], "-1")) {
			sw_1 = 1;
			set_turn(1);
		}
		else if (!strcmp(argv[i], "-3")) {
			sw_3 = 1;
			set_turn(3);
		}
		else if (!strcmp(argv[i], "-l") && (i + 1) < argc) {
			sw_L = 1;
			
			int L = atoi(argv[i + 1]); 
			set_limit(L);
		}
	}
	
	if (sw_f) {
		readFile(filename);
	}
	else { 
		r_start(sw_s, seed);
	}
}

void readFile(char* filename)
{
	
	FILE *f;
	f = fopen(filename, "r");

	//file is invalid if it does not exist
	if (!f) {
		fprintf(stderr, "Invalid filename %s\n", filename);
		exit(1);
		f = stdin;
	}

	char* currentWord;
	char currentLine[MAX_STR_SIZE];

	int flipLine = 0;
	int flip = 0;
	
	set_turn(1);
	set_limit(-1);
	
	short limit = get_limit();
	
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
	
	int numMoves = -1;
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
					return;
				break;

				//RULES CASE
				case 1:

					if (!strcmp(currentWord, "turn")) {

						commandNext = 1;
					}
					else if(commandNext == 1) {

						//Turns the string into an int to be assigned
						
						int temp = atoi(currentWord);
						
						set_turn(temp);
						commandNext = 0;
					}
					else if(!strcmp(currentWord, "unlimited")) {
						set_limit(-1);
					}
					else if (!strcmp(currentWord, "limit")) {

						commandNext = 2;
					}
					else if(commandNext == 2) {

						limit = atoi(currentWord);
						set_limit(limit);
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
							while (i < get_turn())
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
											if (wst.stored[wst.size - 1].val == 13) tabl.cols[endCol].stored[0] = rmv_card(wst.stored, &(wst.size), wst.size - 1);
										}
										
										
										else
										{
											fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
											exit(0);
										}
									}
									else 
									{
										fprintf(stderr, "Move %s is illegal: %d", currentWord, cMoves);
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
										if (tabl.cols[startCol].stored[sPos].val == 13) found = 1;
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
	
	f_start(tabl, fnd, stk, wst);
	
	fflush(stdout);
	free(stk.stored);
	free(wst.stored);
	free(fnd.stored);
	free(allCards);
	free(duplicates);

	fclose(f);
	return;
}

/*
	c1 is source, c2 is destination
	A move is valid if the cards are opposite and the src is 1 larger than the dest
		and if neither card is covered
*/

void setCard(char* str, t_card* card, char covered) {
	(*card).suit = str[1];
	if (str[0] != '_') {
		(*card).val = getCardVal(str[0]);
	}
	else (*card).val = str[0];
	(*card).covered = covered;
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