/*
* ICSI 404
* Alex Kisiel
*/

#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#define MAX_MEMORY 1000
#define STACK_SIZE 64

// Allocates 1000 bytes of memory for the VM
unsigned char MEMORY[MAX_MEMORY];

// CPU Infrastructure
char OP1, OP2, RESULT;
char REGISTERS[15];
unsigned char* currentInstruction;
int currentOpCode;
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
    // Determines the size of the instruction
    if ((MEMORY[memoryIterator] >> 4) == 7)
        size = 4;
    else
        size = 2;

    // Allocates memory for the instruction
    if (!currentInstruction)
        currentInstruction = malloc(sizeof(char)*size);
    else
        currentInstruction = realloc(currentInstruction, sizeof(char)*size);

    // Stores the instruction in 'currentInstruction'
    for (int i=0; i<size; i++) {
        currentInstruction[i] = MEMORY[memoryIterator++];  
    }
}

// Populates OP1 and OP2 registers and implements 'move'
void dispatch() {
    char opCode = currentInstruction[0] >> 4;
    currentOpCode = opCode;
    // '3R'
    if ((opCode > 0) && (opCode < 7)) {
        OP1 = REGISTERS[currentInstruction[0] & 0xF];
        OP2 = REGISTERS[currentInstruction[1] >> 4];
    }
    // BRANCH1
    else if (opCode == 7) {
        if ((currentInstruction[0] & 0xF) < 6) {
            OP1 = REGISTERS[currentInstruction[1] >> 4];
            OP2 = REGISTERS[currentInstruction[1] & 0xF];
        }
    }
    // LOAD/STORE
    else if ((opCode > 7) && (opCode < 10)) {
        OP1 = REGISTERS[currentInstruction[0] & 0xF];
        OP2 = REGISTERS[currentInstruction[1] >> 4];
    }
    // POP
    else if (opCode == 10) {
        OP1 = REGISTERS[currentInstruction[0] & 0xF];
    }
    // MOVE
    else if (opCode == 11) {
        REGISTERS[currentInstruction[0] & 0xF] = currentInstruction[1];
    }
}

// Switch statement that 'does the work' and stores it in RESULT
void execute() {
    switch (currentOpCode) {
        // HALT
        case 0:
            haltFlag = 1;
            break;
        // '3R' INSTRUCTIONS
        case 1:
            RESULT = OP1 + OP2;
            break;
        case 2:
            RESULT = OP1 & OP2;
            break;
        case 3:
            RESULT = OP1 / OP2;
            break;
        case 4:
            RESULT = OP1 * OP2;
            break;
        case 5:
            RESULT = OP1 - OP2;
            break;
        case 6:
            RESULT = OP1 | OP2;
            break;
        // BRANCH INSTRUCTIONS
        case 7:;
            char branchType = (currentInstruction[0] & 0xF);
            // The address to branch to if the condition holds
            int relativeAddress;
            // br1 and br2 have different address sizes
            if (branchType < 6)
                relativeAddress = 2 * (((currentInstruction[2] >> 4) * 4096) + ((currentInstruction[2] & 0xF) * 256) + currentInstruction[3])-4;
            else {
                relativeAddress = 2 * ((currentInstruction[1] >> 4) * 1048575 + (currentInstruction[1] & 0xF) * 65535 + ((currentInstruction[2] >> 4) * 4096) + ((currentInstruction[2] & 0xF) * 256) + (currentInstruction[3] >> 4) + (currentInstruction[3] & 0xF));
            }
            switch (branchType) {
                case 0:
                    if (OP1 < OP2)
                        memoryIterator += relativeAddress;
                    break;
                case 1:
                    if (OP1 <= OP2)
                        memoryIterator += relativeAddress;
                    break;
                case 2:
                    if (OP1 == OP2) {
                        memoryIterator += relativeAddress;
                    }
                    break;
                case 3:
                    if (OP1 != OP2)
                        memoryIterator += relativeAddress;
                    break;
                case 4:
                    if (OP1 > OP2)
                        memoryIterator += relativeAddress;
                    break;
                case 5:
                    if (OP1 >= OP2)
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
            break;
        // LOAD/STORE iNSTRUCTIONS
        case 8:;
            char storedAddress = OP2 + (2 * (currentInstruction[1] & 0xF));
            // An attempt to not break the program
            if (storedAddress > MAX_MEMORY)
                printf("Requested address exceeds the maximum memory allocated for this VM\n");
            else
                OP1 = MEMORY[storedAddress];
            break;
        // STACK INSTRUCTIONS
        case 10:;
            char stackType = currentInstruction[1] >> 4;
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
                    push(OP1);
                    stackPointer--; // Each push/pop should be two bytes
                    break;
                // POP
                case 8:
                    stackPointer++; // Each push/pop should be two bytes
                    RESULT = pop();
                    break;
            }
            break;
        // INTERRUPT
        case 12:
            // Interrupt 1
            if ((currentInstruction[1] & 0xF) == 1) {
                for (int i=maxProgram; i<MAX_MEMORY; i++) {
                    if (MEMORY[i])
                        printf("%d located at memory %d\n", MEMORY[i], i);
                }
            }
            // Interrupt 0
            else {
                for (int i=0; i<15; i++) {
                    if (REGISTERS[i])        
                        printf("Register %d: %d\n", i, REGISTERS[i]);
                }
                break;
            }

    }
}

// Stores the results of the above execution in the proper register/memory location
void store() {
    // '3R' INSTRUCTIONS
    if ((currentOpCode > 0) && (currentOpCode < 7)) {
        REGISTERS[(currentInstruction[1] & 0xF)] = RESULT;
    }
    // LOAD
    else if (currentOpCode == 8) {
        REGISTERS[(currentInstruction[0] & 0xF)] = OP1;
    }
    // STORE
    else if (currentOpCode == 9) {
        char storedAddress = OP2 + (2 * (currentInstruction[1] & 0xF));
        if (storedAddress > MAX_MEMORY)
            printf("Requested address %d exceeds the maximum memory allocated for this VM\n", storedAddress);
        MEMORY[storedAddress] = OP1;
    }
    // POP
    else if ((currentOpCode == 10) && ((currentInstruction[1] >> 4) == 8)) {
        REGISTERS[(currentInstruction[0] & 0xF)] = RESULT;
    }
}

void main(int argc, char* argv[]) {
    
    load(argv[1]);

    while (!haltFlag) {
        fetch();
        dispatch();
        execute();
        store();
    }

    for (int i=0; i<MAX_MEMORY; i++) {
        if (MEMORY[i])
            printf("%d located at memory %d\n", MEMORY[i], i);
    }

    free(currentInstruction);
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