/*
* ICSI 404
* Alex Kisiel
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *words[5];

// Take a string. Split it into different words, putting them in the words array.
void getWords(char *string) {
    int wordCounter = 0;
    char util[strlen(string)];
    strcpy(util, string);
    char *w = strtok (util," \n\r");
    while (w != NULL && wordCounter<5) {
        words[wordCounter] = w;
        w = strtok (NULL, " \n\r");
        wordCounter++;
    }
}

// takes a string and returns register number or -1 if the string doesn't start with "r" or "R"
int getRegister (char* string) {
	if (string[0] != 'R' && string[0] != 'r') return -1;
	return atoi(string+1);
}

// Parses the '3R' instructions
int operationInstruction(char *string, char *bytes, int opno) {
    getWords(string);
    bytes[0] = ((opno << 4) | getRegister(words[1]));
    bytes[1] = (getRegister(words[2]) << 4) | getRegister(words[3]);
    return 2;
}

// Parses branch instructions of type br1
int branchInstruction1(char *string, char *bytes, int branchType) {
    getWords(string);
    bytes[0] = (7 << 4 | branchType);
    bytes[1] = (getRegister(words[1]) << 4) | getRegister(words[2]);
    int address = atoi(words[3]);
    // Assigns the bytes depending on the size of the address
    if (address < 256) {
        bytes[2] = 0;
        bytes[3] = address;
    }
    // Portions off the address number into the correct bytes
    else {
        bytes[2] = (address / 256);
        bytes[3] = (address % 256);
    }
    return 4;
}

// Parses branch instructions of type br2
int branchInstruction2(char *string, char *bytes, int branchType) {
    getWords(string);
    bytes[0] = (7 << 4 | branchType);
    long int address = atol(words[1]);
    // As above, portions off the address number into the correct bytes
    bytes[1] = (address/65535);
    bytes[2] = ((address%65535)/256);
    bytes[3] = ((address%65535)%256);
    return 4;
}

// Parses load/store instructions
int loadStoreInstruction(char *string, char *bytes, int opno) {
    getWords(string);
    bytes[0] = (opno << 4 | getRegister(words[1]));
    bytes[1] = ((getRegister(words[2])) << 4 | atoi(words[3]));
    return 2;
}

// Parses stack instructions
int stackInstruction(char *string, char *bytes, int stackopno) {
    getWords(string);
    bytes[0] = (10 << 4 | getRegister(words[1]));
    bytes[1] = stackopno;
    return 2;
}

// Parses move instructions
int move(char *string, char *bytes) {
    getWords(string);
    bytes[0] = (11 << 4 | getRegister(words[2]));
    bytes[1] = atoi(words[1]);
    return 2;
}

// Parses interrupt instructions
int interrupt(char *string, char *bytes, int interruptNumber) {
    getWords(string);
    bytes[0] = 12 << 4;
    bytes[1] = 0 << 4;
    return 2;
}


// Figure out from the first word which operation we are doing and do it...
int assembleLine(char *string, char *bytes) {
    char *util = malloc(sizeof(char) * strlen(string));
    strcpy(util, string);
    char *firstWord = strtok(util, " \n\r");
    // HALT
    if (strcmp(firstWord, "halt") == 0) {
        // Special 2-byte instruction case where both bytes are 0
        bytes[0] = 0;
        bytes[1] = 0;
        return 2;
    }

    // 3R INSTRUCTIONS

    // ADD
    if (strcmp(firstWord, "add") == 0) {
        return operationInstruction(string, bytes, 1);
    }
    // AND
    if (strcmp(firstWord, "and") == 0) {
        return operationInstruction(string, bytes, 2);
    }
    // DIVIDE
    if (strcmp(firstWord, "divide") == 0) {
        return operationInstruction(string, bytes, 3);
    }
    // MULTIPLY
    if (strcmp(firstWord, "multiply") == 0) {
        return operationInstruction(string, bytes, 4);
    }
    // SUBTRACT
    if (strcmp(firstWord, "subtract") == 0) {
        return operationInstruction(string, bytes, 5);
    }
    // OR
    if (strcmp(firstWord, "or") == 0) {
        return operationInstruction(string, bytes, 6);
    }

    // BRANCH INSTRUCTIONS

    // BRANCHIFLESS
    if (strcmp(firstWord, "branchIfLess") == 0) {
        return branchInstruction1(string, bytes, 0);
    }

    // BRANCHIFLESSOREQUAL
    if (strcmp(firstWord, "branchIfLessOrEqual") == 0) {
        return branchInstruction1(string, bytes, 1);
    }
    // BRANCHIFEQUAL
    if (strcmp(firstWord, "branchIfEqual") == 0) {
        return branchInstruction1(string, bytes, 2);
    }
    // BRANCHIFNOTEQUAL
    if (strcmp(firstWord, "branchIfNotEqual") == 0) {
        return branchInstruction1(string, bytes, 3);
    }
    // BRANCHIFGREATER
    if (strcmp(firstWord, "branchIfGreater") == 0) {
        return branchInstruction1(string, bytes, 4);
    }
    // BRANCHIFGREATEROREQUAL
    if (strcmp(firstWord, "branchIfGreaterOrEqual") == 0) {
        return branchInstruction1(string, bytes, 5);
    }
    // CALL
    if (strcmp(firstWord, "call") == 0) {
        return branchInstruction2(string, bytes, 6);
    }
    // JUMP
    if (strcmp(firstWord, "jump") == 0) {
        return branchInstruction2(string, bytes, 7);
    }

    // LOAD/STORE INSTRUCTIONS

    // LOAD
    if (strcmp(firstWord, "load") == 0) {
        return loadStoreInstruction(string, bytes, 8);
    }
    // STORE
    if (strcmp(firstWord, "store") == 0) {
        return loadStoreInstruction(string, bytes, 9);
    }

    // STACK INSTRUCTIONS

    // POP
    if (strcmp(firstWord, "pop") == 0) {
        return stackInstruction(string, bytes, 128);
    }
    // PUSH
    if (strcmp(firstWord, "push") == 0) {
        return stackInstruction(string, bytes, 64);
    }
    // RETURN
    if (strcmp(firstWord, "return") == 0) {
        bytes[0] = 10 << 4;
        bytes[1] = 0 << 4;
        return 2;
    }
    // MOVE
    if (strcmp(firstWord, "move") == 0) {
        return move(string, bytes);
    }
    // INTERRUPT
    if (strcmp(firstWord, "interrupt") == 0) {
        return interrupt(string, bytes, 12);
    }

    free(util);

    return 0;
}

int main (int argc, char **argv)  {
	if (argc != 3)  {printf ("assemble inputFile outputFile\n"); exit(1); }
	FILE *in = fopen(argv[1],"r");
	if (in == NULL) { printf ("unable to open input file\n"); exit(1); }
	FILE *out = fopen(argv[2],"wb");
	if (out == NULL) { printf ("unable to open output file\n"); exit(1); }

	char inputLine[100];
	while (!feof(in)) {
		if (fgets(inputLine,100,in) != NULL) {
            char bytes[4];
			int outSize = assembleLine(inputLine,bytes);
			fwrite(bytes,outSize,1,out);
		}
	}
	fclose(in);
	fclose(out);
}