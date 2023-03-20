/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure *not* to modify printState or any of the associated functions
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Machine Definitions
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */

// File
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

// Added Lines
extern void cache_init(int blockSize, int numSets, int blocksPerSet);
extern int cache_access(int addr, int write_flag, int write_data);
extern void printStats();
static stateType state;
static int num_mem_accesses = 0;
int mem_access(int addr, int write_flag, int write_data) {
    ++num_mem_accesses;
    if (write_flag) {
        state.mem[addr] = write_data;
    }
    return state.mem[addr];
}
int get_num_mem_accesses(){
	return num_mem_accesses;
}
// Added lines ^^^

void printState(stateType *);

int convertNum(int);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    //stateType state;
    FILE *filePtr;

    // printf("%s \n%s\n%s\n done", argv[0], argv[1], argv[2]);
    if (argc != 5) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* CACHE init */
    int blockSize = atoi(argv[2]);
    int numSets = atoi(argv[3]);
    int blocksPerSet = atoi(argv[4]);
    cache_init(blockSize, numSets, blocksPerSet);

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        // printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    //Your code starts here

    // Set all reg to 0
    for (int i = 0; i < NUMREGS; ++i) {
        state.reg[i] = 0;
    }

    stateType* statePtr = & state;
    // printState(statePtr);

    // Simulate program until halt
    int true = 1;
    int numInstructions = 0;
    while (true == 1) {
        int regA = -1, regB = -1, destR = -1, offset = 0; // fix
        int instruction = cache_access(state.pc, 0, 0); // state.mem[state.pc];
        // printf("instruction: %d\n", instruction);
        int opcode = instruction >> 22; // Add & operation

        if (opcode == 0b000) { // add
            regA = (instruction >> 19) & 0x00000007;
            regB = (instruction >> 16) & 0x00000007;
            destR = (instruction & 0x00000007);

            // ADD contents of regA and regB, store in destR
            state.reg[destR] = state.reg[regA] + state.reg[regB];
        }

        else if (opcode == 0b001) { // nor
            regA = (instruction >> 19) & 0x00000007;
            regB = (instruction >> 16) & 0x00000007;
            destR = (instruction & 0x00000007);

            // NOR contents of regA and regB, store destR (!(a or b))
            state.reg[destR] = ~(state.reg[regA] | state.reg[regB]);
        }

        else if (opcode == 0b010) { // lw
            regA = (instruction >> 19) & 0x00000007;
            regB = (instruction >> 16) & 0x00000007;
            offset = convertNum(instruction & 0x0000ffff);

            // Store contents of memory location (regA + offset) in regB
            state.reg[regB] = cache_access(state.reg[regA] + offset, 0, 0); // state.mem[state.reg[regA] + offset];
        }

        else if (opcode == 0b011) { // sw
            regA = (instruction >> 19) & 0x00000007;
            regB = (instruction >> 16) & 0x00000007;
            offset = convertNum(instruction & 0x0000ffff);

            // Store contents of regB in memory location (regA + offset)
            // state.mem[state.reg[regA] + offset] = state.reg[regB];
            state.numMemory = (state.reg[regA] + offset) > state.numMemory ? state.reg[regA] + offset : state.numMemory;
            cache_access(state.reg[regA] + offset, 1, state.reg[regB]);
        }

        else if (opcode == 0b100) { // beq
            regA = (instruction >> 19) & 0x00000007;
            regB = (instruction >> 16) & 0x00000007;
            offset = convertNum(instruction & 0x0000ffff);

            // If contents of regA == regB, branch to PC+1+offset
            if (state.reg[regA] == state.reg[regB]) {
                state.pc = state.pc + offset; // +1 taken care of at end of loop
            }
        }

        else if (opcode == 0b101) { // jalr
            regA = (instruction >> 19) & 0x00000007;
            regB = (instruction >> 16) & 0x00000007;

            // Store (pc + 1)  into regB
            state.reg[regB] = state.pc + 1;

            // Branch to the address contained in regA
            state.pc = state.reg[regA] - 1; // +1  at end of loop
        }

        else if (opcode == 0b110) { // halt
            // Increment pc, then halt machine
            state.pc += 1;
            numInstructions += 1;
            break;
        }

        else if (opcode == 0b111) { // noop
            ; // do nothing
        }

        else {
            printf("UNRECOGNIZED OPCODE%d", opcode);
            exit(1);
        }
        state.pc += 1;
        numInstructions += 1;
        //printState(statePtr);
        // if (opcode == 0b110) { // halt
        //     printStats();
        // }
    }

    printf("machine halted\n");
    printf("final state of machine:\n");

    printState(statePtr);
    printStats();
    // printf("Main memory words accessed: %d\n", get_num_mem_accesses());

    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    // for (i=0; i<24; i++) {
    //     printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    // }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}

