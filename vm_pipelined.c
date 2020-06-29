/*
* ICSI 404 Assignment 4: Pipelines
* Alex Kisiel
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_MEMORY 1000
#define STACK_SIZE 64

// Allocates 1000 bytes of memory for the VM
unsigned char MEMORY[MAX_MEMORY];

// CPU Infrastructure
// Extra OP and RESULT for pipeline implementation
char OP1_1, OP2_1, OP1_2, OP2_2, RESULT1, RESULT2;
char REGISTERS[15];
// Extra 'currentInstruction' variables for pipeline implementation
unsigned char* currentInstruction1 = NULL;
unsigned char* currentInstruction2 = NULL;
unsigned char* currentInstruction3 = NULL;
unsigned char* currentInstruction4 = NULL; 
// 'Booleans' for double buffering
int onlyFirst = 1, canDispatch1 = 0, canDispatch2 = 0, canEx1 = 0, canEx2 = 0, brValid = 1;
int memoryIterator = 0;
int stackPointer;
int maxProgram = 0;
int haltFlag = 0;

// Reads file into the memory starting at the beginning
void load(char* fileName) {
    FILE* file;
    if ((file = fopen(fileName, "r")) == NULL)
        perror("Can't open the file");
    unsigned char inputLine[4];
	while (!feof(file)) {
		if (fgets(inputLine,2,file) != NULL) {
            for (int i=0; i<strlen(inputLine); i++) {
                MEMORY[memoryIterator++] = inputLine[i];
            }
        }
        if (!strlen(inputLine)) {
            memoryIterator++;
        }
	}

    // I attached the R15 stack to the end of the program in MEMORY.
    // To access it I use a 'stackPointer' initialized to the end of the
    // program memory, which is decremented as items are pushed on and incremented
    // when they are popped. I included some hard-coded warnings if the user pushes
    // too many and starts overwriting the main program memory.

    // The current stack size is 64 bytes, which allows for 32 2-byte pushes
    maxProgram = memoryIterator + STACK_SIZE;
    stackPointer = maxProgram;
    memoryIterator = 0;
    fclose(file);
}

// Initial function declarations
void push(char c);
char pop();

// Read instructions from memory
void fetch() {
    int size;
    onlyFirst = 1;
    // Determines the size of the instruction
    if ((MEMORY[memoryIterator] >> 4) == 7)
        size = 4;
    else
        size = 2;

    // If canDispatch1 is 0, the first instruction is either empty or has already been dispatched
    if (!canDispatch1) {
        // Allocates memory for the instruction
        if (!currentInstruction1)
            currentInstruction1 = malloc(sizeof(char)*size);
        else
            currentInstruction1 = realloc(currentInstruction1, sizeof(char)*size);

        // Stores the instruction in 'currentInstruction1'
        for (int i=0; i<size; i++) {
            currentInstruction1[i] = MEMORY[memoryIterator++];  
        }
        canDispatch1 = 1;
    }
    // If canDispatch1 is 1, the first instruction is ready for dispatch, but has not 
    // been dispatched; fetch reads into the second instruction in this case
    else {
        // Allocates memory for the instruction
        if (!currentInstruction2)
            currentInstruction2 = malloc(sizeof(char)*size);
        else
            currentInstruction2 = realloc(currentInstruction2, sizeof(char)*size);

        // Stores the instruction in 'currentInstruction2'
        for (int i=0; i<size; i++) {
            currentInstruction2[i] = MEMORY[memoryIterator++];  
        }
        canDispatch2 = 1;
        onlyFirst = 0;
    }
}

// Populates OP1 and OP2 registers and implements 'move'
void dispatch() {
    char opCode;

    // Determines which 'currentInstruction' to use
    if (canDispatch1) {
        opCode = currentInstruction1[0] >> 4;
        // '3R' INSTRUCTIONS
        if ((opCode != 7) && (opCode != 10) && (opCode != 11) && (opCode != 12)) {
            OP1_1 = currentInstruction1[0] & 0xF;
            OP2_1 = currentInstruction1[1] >> 4;
        }
        // BRANCH1
        else if (opCode == 7) {
            if ((currentInstruction1[0] & 0xF) < 6) {
                OP1_1 = currentInstruction1[1] >> 4;
                OP2_1 = currentInstruction1[1] & 0xF;
            }
        }
        // LOAD/STORE
        else if ((opCode > 7) && (opCode < 10)) {
            OP1_1 = currentInstruction1[0] & 0xF;
            OP2_1 = currentInstruction1[1] >> 4;
        }
        // POP
        else if (opCode == 10) {
            OP1_1 = currentInstruction1[0] & 0xF;
        }

        // Allocates space for/fills currentInstruction3
        if (!currentInstruction3)
            currentInstruction3 = malloc(sizeof(currentInstruction1));
        else
            currentInstruction3 = realloc(currentInstruction3, sizeof(currentInstruction1));

        // Copies currentInstruction1 into currentInstruction3
        
        for (int i=0; i<sizeof(currentInstruction1); i++) {
            currentInstruction3[i] = currentInstruction1[i];
        }
        if (!(currentInstruction1[0] >> 4)) {
            currentInstruction3[0] = 0; currentInstruction3[1] = 0;
        }
        // Resets the 'canDispatch' boolean, and sets the 'can execute first instruction' flag as true
        canDispatch1 = 0;
        canEx1 = 1;
    }
    else if (canDispatch2) {
        opCode = currentInstruction2[0] >> 4;
        // '3R' INSTRUCTIONS
        if ((opCode != 7) && (opCode != 10) && (opCode != 11) && (opCode != 12)) {
            OP1_2 = currentInstruction2[0] & 0xF;
            OP2_2 = currentInstruction2[1] >> 4;
        }
        // BRANCH1
        else if (opCode == 7) {
            if ((currentInstruction2[0] & 0xF) < 6) {
                OP1_2 = currentInstruction2[1] >> 4;
                OP2_2 = currentInstruction2[1] & 0xF;
            }
        }
        // LOAD/STORE
        else if ((opCode > 7) && (opCode < 10)) {
            OP1_2 = currentInstruction2[0] & 0xF;
            OP2_2 = currentInstruction2[1] >> 4;
        }
        // POP
        else if (opCode == 10) {
            OP1_2 = currentInstruction2[0] & 0xF;
        }

        // Repeats the above processes for currentInstruction4
        if (!currentInstruction4)
            currentInstruction4 = malloc(sizeof(currentInstruction2));
        else
            currentInstruction4 = realloc(currentInstruction4, sizeof(currentInstruction2));

        for (int i=0; i<sizeof(currentInstruction2); i++) {
            currentInstruction4[i] = currentInstruction2[i];
        }
        if (!(currentInstruction2[0] >> 4)) {
            currentInstruction4[0] = 0; currentInstruction4[1] = 0;
        }
        canDispatch2 = 0;
        canEx2 = 1;
    }
}
// Switch statement that 'does the work' and stores it in RESULT
void execute() {
    char opCode, OP1, OP2, stackType, branchType, storedAddress;
    char forwardedOP1, forwardedOP2;
    int relativeAddress, forwardedResult, isSecondInstruction = 0;
    unsigned char** instructionPointer;
    unsigned char** otherInstruction;

    if (!(canEx1 || canEx2))
        return;

    if (canEx1) {
        instructionPointer = &currentInstruction3;
        otherInstruction = &currentInstruction4;
        // Sets the OP registers for the first instruction
        OP1 = OP1_1;
        OP2 = OP2_1;
        // Forwarded RESULT will be the last (opposite) instruction's RESULT 
        forwardedResult = RESULT2;
    }
    else {
        isSecondInstruction = 1;
        instructionPointer = &currentInstruction4;
        otherInstruction = &currentInstruction3;
        OP1 = OP1_2;
        OP2 = OP2_2;
        forwardedResult = RESULT1;
    }

    // If the other instruction in the pipeline was a branch, checks whether the branch was taken
    if (!onlyFirst && ((otherInstruction[0][0] >> 4) == 7)) {
        if (!brValid) {
            return;
        }
    }
    else
        brValid = 1;

    // The default OP1 and OP2 operators
    forwardedOP1 = REGISTERS[OP1];
    forwardedOP2 = REGISTERS[OP2];
    
    // Register Forwarding:
    // If the previous instruction was '3R', 'LOAD', or 'POP', uses the 
    // previous RESULT instead of the respective RESULT[OP] value

    // Previous instruction was '3R'
    if (!onlyFirst && ((otherInstruction[0][0] >> 4) > 0) && (otherInstruction[0][0] >>4) < 7) {
        if ((otherInstruction[0][1] & 0xF) == OP1)
            forwardedOP1 = forwardedResult;
        else if ((otherInstruction[0][1] &0xF) == OP2)
            forwardedOP2 = forwardedResult;
    }
    // Previous instruction was 'LOAD'
    if (!onlyFirst && (otherInstruction[0][0] >> 4) == 8) {
        if ((otherInstruction[0][0] & 0xF) == OP1)
            forwardedOP1 = forwardedResult;
        else if ((otherInstruction[0][0] & 0xF) == OP2)
            forwardedOP2 = forwardedResult;
    }
    // Previous instruction was 'POP'
    if (!onlyFirst && ((otherInstruction[0][0] >> 4) == 10) && ((otherInstruction[0][1] >> 4) == 8)) {
        if ((otherInstruction[0][0] & 0xF) == OP1)
            forwardedOP1 = forwardedResult;
        if ((otherInstruction[0][0] & 0xF) == OP2)
            forwardedOP2 = forwardedResult;
    }

    opCode = instructionPointer[0][0] >> 4;

    switch (opCode) {
        // HALT
        case 0:
            haltFlag = 1;
            break;
        // '3R' INSTRUCTIONS
        case 1:
            if (canEx1)
                RESULT1 = forwardedOP1 + forwardedOP2;
            else
                RESULT2 = forwardedOP1 + forwardedOP2;
            break;
        case 2:
            if (canEx1)
                RESULT1 = forwardedOP1 & forwardedOP2;
            else
                RESULT2 = forwardedOP1 & forwardedOP2;
            break;
        case 3:
            if (canEx1)
                RESULT1 = forwardedOP1 / forwardedOP2;
            else
                RESULT2 = forwardedOP1 / forwardedOP2;
            break;
        case 4:
            if (canEx1)
                RESULT1 = forwardedOP1 * forwardedOP2;
            else
                RESULT2 = forwardedOP1 * forwardedOP2;
            break;
        case 5:
            if (canEx1)
                RESULT1 = forwardedOP1 - forwardedOP2;
            else
                RESULT2 = forwardedOP1 - forwardedOP2;
            break;
        case 6:
            if (canEx1)
                RESULT1 = forwardedOP1 | forwardedOP2;
            else
                RESULT2 = forwardedOP1 | forwardedOP2;
            break;

        // BRANCH INSTRUCTIONS

        case 7:
            branchType = instructionPointer[0][0] & 0xF;
            int tempMemoryIterator = memoryIterator;
            if (branchType < 6) {
                relativeAddress = 2 * (((instructionPointer[0][2] >> 4) * 4096) + ((instructionPointer[0][2] & 0xF) * 256) + instructionPointer[0][3])-4;
            }
            else {
                relativeAddress = 2 * ((instructionPointer[0][1] >> 4) * 1048575 + (instructionPointer[0][1] & 0xF) * 65535 + ((instructionPointer[0][2] >> 4) * 4096) + ((instructionPointer[0][2] & 0xF) * 256) + (instructionPointer[0][3] >> 4) + (instructionPointer[0][3] & 0xF));
            }
            switch (branchType) {
                case 0:
                    if (forwardedOP1 < forwardedOP2)
                        memoryIterator += relativeAddress;
                    break;
                case 1:
                    if (forwardedOP1 <= forwardedOP2)
                        memoryIterator += relativeAddress;
                    break;
                case 2:
                    if (forwardedOP1 == forwardedOP2) {
                        memoryIterator += relativeAddress;
                    }
                    break;
                case 3:
                    if (forwardedOP1 != forwardedOP2)
                        memoryIterator += relativeAddress;
                    break;
                case 4:
                    if (forwardedOP1 > forwardedOP2)
                        memoryIterator += relativeAddress;
                    break;
                case 5:
                    if (forwardedOP1 >= forwardedOP2)
                        memoryIterator += relativeAddress;
                    break;

                // CALL
                case 6:
                    // Parses/pushes the address of the next instruction to the stack
                    push((((memoryIterator / 16) << 4) | ((memoryIterator % 16))));
                    push((((memoryIterator / 4096) << 4) | (((memoryIterator % 4096) / 256))));
                    memoryIterator = relativeAddress;
                    break;
                // JUMP
                case 7:
                    memoryIterator = relativeAddress;
                    break;
            }

            // If the branch was taken (i.e. the memoryIterator has changed), and the branch wasn't in the second instruction, the pipeline is not valid
            if ((tempMemoryIterator != memoryIterator) && (!isSecondInstruction)) {
                brValid = 0;
                canDispatch1 = 0;
                canDispatch2 = 0;
            }
            break;
        // LOAD/STORE iNSTRUCTIONS
        case 8:
            storedAddress = forwardedOP2 + (2 * (instructionPointer[0][1] & 0xF));
            // An attempt to not break the program
            if (storedAddress > MAX_MEMORY)
                printf("Requested address exceeds the maximum memory allocated for this VM\n");
            else {
                if (canEx1)
                    RESULT1 = MEMORY[storedAddress];
                else
                    RESULT2 = MEMORY[storedAddress];

            }
            break;
        // STACK INSTRUCTIONS
        case 10:
            stackType = instructionPointer[0][1] >> 4;
            switch (stackType) {
                // RETURN
                case 0:;
                    // Pops/parses the address
                    char address0 = pop();
                    char address1 = pop();
                    int addr = ((address0 >> 4) * 4096) + ((address0 & 0xF) * 256);
                    addr += ((address1 >> 4) * 16) + (address1 & 0xF);
                    memoryIterator = addr;
                    break;
                // PUSH
                case 4:
                    push(forwardedOP1);
                    stackPointer--; // Each push/pop should be two bytes
                    break;
                // POP
                case 8:
                    stackPointer++; // Each push/pop should be two bytes
                    if (canEx1)
                        RESULT1 = pop();
                    else
                        RESULT2 = pop();
                    break;
            }
            break;
    }

    // Resets the respective boolean
    if (canEx1)
        canEx1 = 0;
    else if (canEx2)
        canEx2 = 0;
}

// Stores the results of the above execution in the proper register/memory location
void store() {
    char OP1, OP2, RESULT;
    unsigned char** instructionPointer;

    if (canEx1 && canEx2)
        return;

    if (!brValid) {
        return;
    }

    // Determines which 'currentInstruction' will be used
    if (!canEx1) {
        instructionPointer = &currentInstruction3;
        OP1 = OP1_1;
        OP2 = OP2_1;
        RESULT = RESULT1;
        canEx1 = 1;
    }
    else if (!canEx2) {
        instructionPointer = &currentInstruction4;
        OP1 = OP1_2;
        OP2 = OP2_2;
        RESULT = RESULT2;
        canEx2 = 1;
    }
    
    int opCode = instructionPointer[0][0] >> 4;

    // '3R' INSTRUCTIONS
    if ((opCode > 0) && (opCode < 7)) {
        REGISTERS[(instructionPointer[0][1] & 0xF)] = RESULT;
    }
    // LOAD
    else if (opCode == 8) {
        REGISTERS[instructionPointer[0][0] & 0xF] = RESULT;
    }
    // STORE
    else if (opCode == 9) {
        char storedAddress = REGISTERS[OP2] + (2 * (instructionPointer[0][1] & 0xF));
        if (storedAddress > MAX_MEMORY)
            printf("Requested address %d exceeds the maximum memory allocated for this VM\n", storedAddress);
        MEMORY[storedAddress] = REGISTERS[OP1];
    }
    // POP
    else if ((opCode == 10) && ((instructionPointer[0][1] >> 4) == 8)) {
        REGISTERS[(instructionPointer[0][0] & 0xF)] = RESULT;
    }
    // MOVE
    else if (opCode == 11) {
        REGISTERS[instructionPointer[0][0] & 0xF] = instructionPointer[0][1];
    }
    // INTERRUPTS
    else if (opCode == 12) {
        // Interrupt 1
        if (instructionPointer[0][1] & 0xF) {
            for (int i=0; i<MAX_MEMORY; i++) {
                printf("%d located at memory %d\n", MEMORY[i], i);
            }
        }
        // Interrupt 0
        else {
            for (int i=0; i<15; i++) {
                if (REGISTERS[i])        
                    printf("Register %d: %d\n", i, REGISTERS[i]);
            }
        }
    }
}

void main(int argc, char* argv[]) {
    
    load(argv[1]);

    while (!haltFlag) {
        // Wasn't sure how to implement it so that they could be called out of order and still function (didn't know how to force one to wait for another without forks/the like).
        // Tested both normally in order (as it is currently) and doubled up (remove the comments; each method is called twice in a row)
                fetch();
                // fetch();
                dispatch();
                // dispatch();
                execute();
                // execute();
                store();
                // store();
    }
    free(currentInstruction1);
    free(currentInstruction2);
    free(currentInstruction3);
    free(currentInstruction4);
}

// A couple helper methods for the stack

void push(char pushee) {
    if (stackPointer-1 <= maxProgram-STACK_SIZE)
        printf("Too many items on the stack -- program is being overwritten\n");
    else
        MEMORY[--stackPointer] = pushee;
}

char pop() {
    return MEMORY[stackPointer++];
}