/*
 * solitaire_p1.c
 *
 *  Created on: Feb 7, 2020
 *      Author: Cameron Isbell
 */
//START DECLARATIONS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_SIZE 100
#define MAX_CARDS 52
#define CARD_LEN 3


int readFile(char* filename);
int verifyValue(char val);
int verifySuit(char suit);
char contains(char* val, char list[MAX_CARDS][CARD_LEN], int numItems);
char** getWords(char* fullLine, int* words);

typedef struct cards {

	int size;
	char stored[MAX_CARDS][CARD_LEN];

} cards;

void handleDuplicate(cards* duplicates, cards* allCards, char* toCheck);


//END DECLARATIONS

int main(int argc, char* argv[])
{
	readFile("testFile.txt");
}

int readFile(char* filename)
{
	FILE *f;

	f = fopen(filename, "r");

	//file is invalid if it does not exist
	if (!f) {
		f = stdin;
	}

	char currentWord[MAX_STR_SIZE];
	char currentLine[MAX_STR_SIZE];

	int flipLine = 0;
	int flip = 0;
	int turn = 0;
	int limit = 0;
	int line = 0;
	int tracker = 0;
	int commandNext = 0;
	int numWords = 0;

	cards covered;
	covered.size = 0;

	cards uncovered;
	uncovered.size = 0;

	cards duplicates;
	duplicates.size = 0;

	cards waste;
	waste.size = 0;

	cards stock;
	stock.size = 0;

	cards foundations;
	foundations.size = 0;

	cards allCards;
	allCards.size = 0;

	//Intialize our cards to be empty
	for (int ia = 0; ia < MAX_CARDS; ia++) {
		strcpy(covered.stored[ia], "\0");
		strcpy(uncovered.stored[ia], "\0");
		strcpy(duplicates.stored[ia], "\0");
		strcpy(waste.stored[ia], "\0");
		strcpy(stock.stored[ia], "\0");
		strcpy(foundations.stored[ia], "\0");
		strcpy(allCards.stored[ia], "\0");
	}

	//runs until the file is completed, reads a line from the file at a time
	while (fgets(currentLine, MAX_STR_SIZE, f)) {
		//create a copy of the current line of the file, and use it to split into an array
		char tempLine[MAX_STR_SIZE];
		strcpy(tempLine, currentLine);

		char** wordList;
		wordList = getWords(tempLine, &numWords);

		if (!wordList) continue;

		for(int i = 0; i < numWords; i++) {
			strcpy(currentWord, wordList[i]);
			if(currentWord[0] == '\0') continue;

			//strcmp returns 0 if the strings are equal, so result must be inverted
			if (!strcmp(currentWord,"RULES:")) {

				if (tracker != 0) {
					fprintf(stderr, "Incorrect Instruction: Expected RULES:, line %d", line);
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
			else if (!strcmp(currentWord, "MOVES:") &&       tracker == 4)	tracker = 6;

			switch(tracker) {


				//If we enter here, then the file doesn't begin with RULES, making it invalid
				case 0:
					return 0;
				break;

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

				case 2:

					//if the current word is valid for the foundation, add it to the foundations array
					strcpy(foundations.stored[foundations.size], currentWord);
					foundations.size++;

					if (currentWord[0] != '_') {
						handleDuplicate(&duplicates, &allCards, currentWord);
					}

				break;

				case 3:
					//Covered / uncovered flip resets on each line, so if the line has changed, reset flipLine
					if (flip && flipLine != line) {
						flipLine = 0;
						flip = 0;
					}

					//Pipe sets flip to 1, switches from covered to uncovered
					if (!strcmp(currentWord, "|")) {
						flipLine = line;
						flip = 1;
					}
					else if (!flip) {
						strcpy(covered.stored[covered.size], currentWord);
						covered.size++;

						handleDuplicate(&duplicates, &allCards, currentWord);
					}

					else if (flip) {
						strcpy(uncovered.stored[uncovered.size], currentWord);
						uncovered.size++;

						handleDuplicate(&duplicates, &allCards, currentWord);
					}

				break;

				case 4:
					//cards go in waste first, then stock after a pipe
					if (!strcmp(currentWord, "|")) {
						flip = 1;
					}
					else if (!flip) {
						strcpy(waste.stored[waste.size], currentWord);
						waste.size++;

						handleDuplicate(&duplicates, &allCards, currentWord);

					}
					else if (flip) {
						strcpy(stock.stored[stock.size], currentWord);
						stock.size++;

						handleDuplicate(&duplicates, &allCards, currentWord);
					}

				break;

				//Implement at a later stage of the project
				//WILL NEVER REACH CASE 5 WITH CURRENT IMPLEMENATION
				case 5:
					tracker++;

				break;

				case 6:

					if (duplicates.size > 0) {
						fprintf(stderr, "Duplicate cards: ");
						int x;
						for (x = 0; x < duplicates.size; x++) {
							fprintf(stderr, "%s", duplicates.stored[x]);
						}
						return 0;
					}
					else{
						fprintf(stdout, "Input file is valid\n");
						fprintf(stdout, "%d covered cards\n%d stock cards\n%d waste cards\n", covered.size, stock.size, waste.size);

					}
					fprintf(stdout, "\n");
					fflush(stdin);
				break;


			}

		}
		line++;
	}
	fclose(f);
	return 1;
}

/*
* Handles duplicates by checking if the card is already listed in allCards
* Will always add a duplicate to the list of all cards so it checks for multiple
* duplicates, and because the output will be invalid anyway
*/
void handleDuplicate(cards* duplicates, cards* allCards, char* toCheck)
{
	//printf("%s\n", toCheck);
	if (contains(toCheck, allCards -> stored, allCards -> size)) {
		strcpy(duplicates -> stored[duplicates -> size], toCheck);
		duplicates -> size++;
	}
	strcpy(allCards -> stored[allCards -> size], toCheck);
	allCards -> size++;
}

/*
 * If val is in the list of strings, returns the index.
 * Otherwise, return 0
 */
char contains(char* val, char list[MAX_CARDS][CARD_LEN], int numItems)
{
	int i = 0;
	for (i = 0; i < numItems; i++) {
		if (!strcmp(val, list[i])) return 1;
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
		strcpy(wordList[i], current);
		current = strtok(NULL, delimiter);
		i++;
	}

	*words = numWords;
	return wordList;
}

