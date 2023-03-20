/*
 * EECS 370, University of Michigan
 * Project 3: LC-2K Pipeline Simulator
 * Instructions are found in the project spec.
 * Make sure NOT to modify printState or any of the associated functions
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Machine Definitions
#define NUMMEMORY 65536 // maximum number of data words in memory
#define NUMREGS 8 // number of machine registers

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 // will not implemented for Project 3
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION (NOOP << 22)

typedef struct IFIDStruct {
	int instr;
	int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
	int instr;
	int pcPlus1;
	int readRegA;
	int readRegB;
	int offset;
} IDEXType;

typedef struct EXMEMStruct {
	int instr;
	int branchTarget;
    int eq;
	int aluResult;
	int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
	int instr;
	int writeData;
} MEMWBType;

typedef struct WBENDStruct {
	int instr;
	int writeData;
} WBENDType;

typedef struct stateStruct {
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles; // number of cycles run so far
} stateType;

static inline int opcode(int instruction) {
    return instruction>>22;
}

static inline int field0(int instruction) {
    return (instruction>>19) & 0x7;
}

static inline int field1(int instruction) {
    return (instruction>>16) & 0x7;
}

static inline int field2(int instruction) {
    return instruction & 0xFFFF;
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) {
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

void printState(stateType*);
void printInstruction(int);
void readMachineCode(stateType*, char*);

int main(int argc, char *argv[]) {
    stateType state, newState;
    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    readMachineCode(&state, argv[1]);

    // Initialize state
    state.cycles = 0;
    state.IFID.instr = 0x1c00000;
    state.IDEX.instr = 0x1c00000;
    state.EXMEM.instr = 0x1c00000;
    state.MEMWB.instr = 0x1c00000;
    state.WBEND.instr = 0x1c00000;

    // make everything noop
    // state = {state.pc, state.instrMem, state.dataMem, state.reg, state.numMemory, {0x1c00000,0}, {0x1c00000,0,0,0,0}, {0x1c00000,0,0,0,0}, {0x1c00000,0}, {0x1c00000, 0}, state.cycles};

    while (opcode(state.MEMWB.instr) != HALT) {
        printState(&state);

        newState = state;
        newState.cycles++;

        int halt = 0;

        /* ---------------------- IF stage --------------------- */
        IFIDType IFIDReg = { state.instrMem[state.pc], state.pc + 1 };
        newState.IFID = IFIDReg;

        /* ---------------------- ID stage --------------------- */
        IDEXType IDEXReg;

        // STALLING (if prev instruction = lw and destReg of lw = regA or regB)
        // [TODO]: consider lw followed by lw
        // if (opcode(state.IDEX.instr) == 2 && (field1(state.IDEX.instr) == field0(state.IFID.instr) || field1(state.IDEX.instr) == field1(state.IFID.instr))) {
        //     // Set EX Reg to be noop
        //     IDEXReg.instr = 0x1c00000;
        //     IDEXReg.pcPlus1 = state.IFID.pcPlus1;

        //     // Reset IF Reg
        //     IFIDType IFIDReg = { state.instrMem[state.pc - 1], state.pc };
        //     newState.IFID = IFIDReg;

        //     // state.pc - 1, so pc stays the same?
        //     state.pc--;
        //     halt = 1;
        // }
        if (opcode(state.IDEX.instr) == 2 && (((opcode(state.IFID.instr) == 0 || opcode(state.IFID.instr) == 1 || opcode(state.IFID.instr) == 3 || opcode(state.IFID.instr) == 4)
                                                && (field1(state.IDEX.instr) == field0(state.IFID.instr) || field1(state.IDEX.instr) == field1(state.IFID.instr)))
                                                || ((opcode(state.IFID.instr) == 2)
                                                && (field1(state.IDEX.instr) == field0(state.IFID.instr))))) {
            // Set EX Reg to be noop
            IDEXReg.instr = 0x1c00000;
            IDEXReg.pcPlus1 = state.IFID.pcPlus1;

            // Reset IF Reg
            IFIDType IFIDReg = { state.instrMem[state.pc - 1], state.pc };
            newState.IFID = IFIDReg;

            // state.pc - 1, so pc stays the same?
            state.pc--;
            halt = 1;
        }
        else {
            IDEXReg.instr = state.IFID.instr;
            IDEXReg.pcPlus1 = state.IFID.pcPlus1;
            IDEXReg.readRegA = state.reg[field0(state.IFID.instr)];
            IDEXReg.readRegB = state.reg[field1(state.IFID.instr)];
            IDEXReg.offset = convertNum(field2(state.IFID.instr)); // sign extend
        }
        newState.IDEX = IDEXReg;

        /* ---------------------- EX stage --------------------- */
        // DETECT HAZARDS //
        int regA = field0(state.IDEX.instr); // RegA #
        int regB = field1(state.IDEX.instr); // RegB #
        int regAVal = state.IDEX.readRegA;   // RegA value
        int regBVal = state.IDEX.readRegB;   // RegB value
        
        // I am add/nor/beq/sw //
        if (opcode(state.IDEX.instr) == 0 || opcode(state.IDEX.instr) == 1 || opcode(state.IDEX.instr) == 3 || opcode(state.IDEX.instr) == 4) {
            /* -- distance 3 dependency MEMWB -- */
            if (opcode(state.WBEND.instr) == 0 || opcode(state.WBEND.instr) == 1) { // add/nor
                int d3DestReg = field2(state.WBEND.instr);
                regAVal = (regA == d3DestReg) ? state.WBEND.writeData : regAVal;
                regBVal = (regB == d3DestReg) ? state.WBEND.writeData : regBVal;
            }
            else if (opcode(state.WBEND.instr) == 2) {                              // lw
                int d3DestReg = field1(state.WBEND.instr);
                regAVal = (regA == d3DestReg) ? state.WBEND.writeData : regAVal;
                regBVal = (regB == d3DestReg) ? state.WBEND.writeData : regBVal;
            }      

            /* -- distance 2 dependency MEMWB -- */
            if (opcode(state.MEMWB.instr) == 0 || opcode(state.MEMWB.instr) == 1) { // add/nor
                int d2DestReg = field2(state.MEMWB.instr);
                regAVal = (regA == d2DestReg) ? state.MEMWB.writeData : regAVal;
                regBVal = (regB == d2DestReg) ? state.MEMWB.writeData : regBVal;
            }
            else if (opcode(state.MEMWB.instr) == 2) {                              // lw
                int d2DestReg = field1(state.MEMWB.instr);
                regAVal = (regA == d2DestReg) ? state.MEMWB.writeData : regAVal;
                regBVal = (regB == d2DestReg) ? state.MEMWB.writeData : regBVal;
            }
            
            /* -- distance 1 dependency EXMEM -- */
            if (opcode(state.EXMEM.instr) == 0 || opcode(state.EXMEM.instr) == 1) { // add/nor
                int d1DestReg = field2(state.EXMEM.instr);
                regAVal = (regA == d1DestReg) ? state.EXMEM.aluResult : regAVal;
                regBVal = (regB == d1DestReg) ? state.EXMEM.aluResult : regBVal;
            }
        }

        // I am lw //
        else if (opcode(state.IDEX.instr) == 2) {
            /* -- distance 3 dependency MEMWB -- */
            if (opcode(state.WBEND.instr) == 0 || opcode(state.WBEND.instr) == 1) { // add/nor
                int d3DestReg = field2(state.WBEND.instr);
                regAVal = (regA == d3DestReg) ? state.WBEND.writeData : regAVal;
            }
            else if (opcode(state.WBEND.instr) == 2) {                               // lw
                int d3DestReg = field1(state.WBEND.instr);
                regAVal = (regA == d3DestReg) ? state.WBEND.writeData : regAVal;
            }

            /* -- distance 2 dependency MEMWB -- */
            if (opcode(state.MEMWB.instr) == 0 || opcode(state.MEMWB.instr) == 1) { // add/nor
                int d2DestReg = field2(state.MEMWB.instr);
                regAVal = (regA == d2DestReg) ? state.MEMWB.writeData : regAVal;
            }
            else if (opcode(state.MEMWB.instr) == 2) {                              // lw
                int d2DestReg = field1(state.MEMWB.instr);
                regAVal = (regA == d2DestReg) ? state.MEMWB.writeData : regAVal;
            }

            /* -- distance 1 dependency EXMEM -- */
            if (opcode(state.EXMEM.instr) == 0 || opcode(state.EXMEM.instr) == 1) { // add/nor
                int d1DestReg = field2(state.EXMEM.instr);
                regAVal = (regA == d1DestReg) ? state.EXMEM.aluResult : regAVal;
            }
            else if (opcode(state.MEMWB.instr) == 2) {                              // lw
                int d2DestReg = field1(state.MEMWB.instr);
                regAVal = (regA == d2DestReg) ? state.MEMWB.writeData : regAVal;
            }
        }

        EXMEMType EXMEMReg;
        EXMEMReg.instr = state.IDEX.instr;
        EXMEMReg.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
        EXMEMReg.eq = (regAVal == regBVal);

        // lw or sw
        if (opcode(EXMEMReg.instr) == 2 || opcode(EXMEMReg.instr) == 3) {
            EXMEMReg.aluResult = regAVal + state.IDEX.offset;
        }
        // add, nor, beq, halt, noop
        else {
            if (opcode(EXMEMReg.instr) == 0) { // add
                EXMEMReg.aluResult = regAVal + regBVal;
            }
            else if (opcode(EXMEMReg.instr) == 1) { // nor
                EXMEMReg.aluResult = ~(regAVal | regBVal);
            }
            // beq, halt, noop doesn't matter
        }

        EXMEMReg.readRegB = regBVal;
        newState.EXMEM = EXMEMReg;

        /* --------------------- MEM stage --------------------- */
        MEMWBType MEMWBReg;
        MEMWBReg.instr = state.EXMEM.instr;

        // lw
        if (opcode(MEMWBReg.instr) == 2) {
            MEMWBReg.writeData = state.dataMem[state.EXMEM.aluResult]; // what data we are writing
        }
        // sw
        else if (opcode(MEMWBReg.instr) == 3) {
            newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
        }
        // beq
        else if (opcode(MEMWBReg.instr) == 4) {
            if (state.EXMEM.eq == 1) { // branch
                newState.EXMEM.instr = 0x1c00000;
                newState.IDEX.instr = 0x1c00000;
                newState.IFID.instr = 0x1c00000;
                newState.pc = state.EXMEM.branchTarget - 1;
                // state = newState;
            }
        }
        // add, nor        
        else {
            MEMWBReg.writeData = state.EXMEM.aluResult;
        }
        newState.MEMWB = MEMWBReg;

        /* ---------------------- WB stage --------------------- */
        WBENDType WBENDReg;
        WBENDReg.instr = state.MEMWB.instr;
        WBENDReg.writeData = state.MEMWB.writeData;
        newState.WBEND = WBENDReg;

        // lw
        if (opcode(WBENDReg.instr) == 2) {
            newState.reg[field1(WBENDReg.instr)] = WBENDReg.writeData;
        }
        // add, nor
        else if (opcode(WBENDReg.instr) == 0 || opcode(WBENDReg.instr) == 1) {
            newState.reg[field2(WBENDReg.instr)] = WBENDReg.writeData;
        }
        // beq, sw
        // do nothing

        /* ------------------------ END ------------------------ */
        if (halt == 1) {
            newState.pc = ++state.pc;
        }
        state = newState; /* this is the last statement before end of the loop. It marks the end 
        of the cycle and updates the current state with the values calculated in this cycle */
        if (halt == 0) {
            state.pc++;
        }
    }
    printf("machine halted\n");
    printf("total of %d cycles executed\n", state.cycles);
    printf("final state of machine:\n");
    printState(&state);
}

// int main(int argc, char *argv[]) {
//     stateType state, newState;
//     if (argc != 2) {
//         printf("error: usage: %s <machine-code file>\n", argv[0]);
//         exit(1);
//     }
//     readMachineCode(&state, argv[1]);

//     // Initialize state
//     state.cycles = 0;
//     state.IFID.instr = 0x1c00000;
//     state.IDEX.instr = 0x1c00000;
//     state.EXMEM.instr = 0x1c00000;
//     state.MEMWB.instr = 0x1c00000;
//     state.WBEND.instr = 0x1c00000;

//     // make everything noop
//     // state = {state.pc, state.instrMem, state.dataMem, state.reg, state.numMemory, {0x1c00000,0}, {0x1c00000,0,0,0,0}, {0x1c00000,0,0,0,0}, {0x1c00000,0}, {0x1c00000, 0}, state.cycles};

//     while (opcode(state.MEMWB.instr) != HALT) {
//         printState(&state);

//         newState = state;
//         newState.cycles++;

//         /* ---------------------- IF stage --------------------- */
//         IFIDType IFIDReg = { state.instrMem[state.pc], state.pc + 1 };
//         newState.IFID = IFIDReg;

//         /* ---------------------- ID stage --------------------- */
//         IDEXType IDEXReg;
//         IDEXReg.instr = state.IFID.instr;
//         IDEXReg.pcPlus1 = state.IFID.pcPlus1;
//         IDEXReg.readRegA = state.reg[field0(state.IFID.instr)];
//         IDEXReg.readRegB = state.reg[field1(state.IFID.instr)];
//         IDEXReg.offset = convertNum(field2(state.IFID.instr)); // sign extend
//         newState.IDEX = IDEXReg;

//         /* ---------------------- EX stage --------------------- */
//         EXMEMType EXMEMReg;
//         EXMEMReg.instr = state.IDEX.instr;
//         EXMEMReg.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
//         EXMEMReg.eq = (state.IDEX.readRegA == state.IDEX.readRegB);

//         // lw or sw
//         if (opcode(EXMEMReg.instr) == 2 || opcode(EXMEMReg.instr) == 3) {
//             // EXMEMReg.eq = (state.IDEX.readRegA == state.IDEX.offset);
//             EXMEMReg.aluResult = state.IDEX.readRegA + state.IDEX.offset;
//         }
//         // add, nor, beq, halt, noop
//         else {
//             // EXMEMReg.eq = (state.IDEX.readRegA == state.IDEX.readRegB);
//             if (opcode(EXMEMReg.instr) == 0) { // add
//                 EXMEMReg.aluResult = state.IDEX.readRegA + state.IDEX.readRegB;
//             }
//             else if (opcode(EXMEMReg.instr) == 1) { // nor
//                 EXMEMReg.aluResult = ~(state.IDEX.readRegA | state.IDEX.readRegB);
//             }
//             // beq, halt, noop doesn't matter
//         }

//         EXMEMReg.readRegB = state.IDEX.readRegB;
//         newState.EXMEM = EXMEMReg;

//         /* --------------------- MEM stage --------------------- */
//         MEMWBType MEMWBReg;
//         MEMWBReg.instr = state.EXMEM.instr;

//         // lw
//         if (opcode(MEMWBReg.instr) == 2) {
//             MEMWBReg.writeData = state.dataMem[state.EXMEM.aluResult]; // what data we are writing
//         }
//         // sw
//         else if (opcode(MEMWBReg.instr) == 3) {
//             newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
//         }
//         // beq
//         else if (opcode(MEMWBReg.instr) == 4) {
//             if (state.EXMEM.eq == 1) { // branch
//                 newState.EXMEM.instr = 0x1c00000;
//                 newState.IDEX.instr = 0x1c00000;
//                 newState.IFID.instr = 0x1c00000;
//                 newState.pc = state.EXMEM.branchTarget - 1;
//                 // state = newState;
//             }
//         }        
//         else {
//             MEMWBReg.writeData = state.EXMEM.aluResult;
//         }
//         newState.MEMWB = MEMWBReg;

//         /* ---------------------- WB stage --------------------- */
//         WBENDType WBENDReg;
//         WBENDReg.instr = state.MEMWB.instr;
//         WBENDReg.writeData = state.MEMWB.writeData;
//         newState.WBEND = WBENDReg;

//         // lw
//         if (opcode(WBENDReg.instr) == 2) {
//             newState.reg[field1(WBENDReg.instr)] = WBENDReg.writeData;
//         }
//         // add, nor
//         else if (opcode(WBENDReg.instr) == 0 || opcode(WBENDReg.instr) == 1) {
//             newState.reg[field2(WBENDReg.instr)] = WBENDReg.writeData;
//         }
//         // beq, sw
//         // do nothing

//         /* ------------------------ END ------------------------ */
//         state = newState; /* this is the last statement before end of the loop. It marks the end 
//         of the cycle and updates the current state with the values calculated in this cycle */
//         state.pc++;
//     }
//     printf("machine halted\n");
//     printf("total of %d cycles executed\n", state.cycles);
//     printf("final state of machine:\n");
//     printState(&state);
// }

void printInstruction(int instr) {
    switch (opcode(instr)) {
        case ADD:
            printf("add");
            break;
        case NOR:
            printf("nor");
            break;
        case LW:
            printf("lw");
            break;
        case SW:
            printf("sw");
            break;
        case BEQ:
            printf("beq");
            break;
        case JALR:
            printf("jalr");
            break;
        case HALT:
            printf("halt");
            break;
        case NOOP:
            printf("noop");
            break;
        default:
            printf(".fill %d", instr);
            return;
    }
    printf(" %d %d %d", field0(instr), field1(instr), field2(instr));
}

void printState(stateType *statePtr) {
    printf("\n@@@\n");
    printf("state before cycle %d starts:\n", statePtr->cycles);
    printf("\tpc = %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (int i=0; i<statePtr->numMemory; ++i) {
        printf("\t\tdataMem[ %d ] = %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i=0; i<NUMREGS; ++i) {
        printf("\t\treg[ %d ] = %d\n", i, statePtr->reg[i]);
    }

    // IF/ID
    printf("\tIF/ID pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IFID.instr);
    printInstruction(statePtr->IFID.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IFID.pcPlus1);
    if(opcode(statePtr->IFID.instr) == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    
    // ID/EX
    int idexOp = opcode(statePtr->IDEX.instr);
    printf("\tID/EX pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->IDEX.instr);
    printInstruction(statePtr->IDEX.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IDEX.pcPlus1);
    if(idexOp == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegA = %d", statePtr->IDEX.readRegA);
    if (idexOp >= HALT || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->IDEX.readRegB);
    if(idexOp == LW || idexOp > BEQ || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\toffset = %d", statePtr->IDEX.offset);
    if (idexOp != LW && idexOp != SW && idexOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // EX/MEM
    int exmemOp = opcode(statePtr->EXMEM.instr);
    printf("\tEX/MEM pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->EXMEM.instr);
    printInstruction(statePtr->EXMEM.instr);
    printf(" )\n");
    printf("\t\tbranchTarget %d", statePtr->EXMEM.branchTarget);
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\teq ? %s", (statePtr->EXMEM.eq ? "True" : "False"));
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\taluResult = %d", statePtr->EXMEM.aluResult);
    if (exmemOp > SW || exmemOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\treadRegB = %d", statePtr->EXMEM.readRegB);
    if (exmemOp != SW) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // MEM/WB
	int memwbOp = opcode(statePtr->MEMWB.instr);
    printf("\tMEM/WB pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->MEMWB.instr);
    printInstruction(statePtr->MEMWB.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->MEMWB.writeData);
    if (memwbOp >= SW || memwbOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");     

    // WB/END
	int wbendOp = opcode(statePtr->WBEND.instr);
    printf("\tWB/END pipeline register:\n");
    printf("\t\tinstruction = %d ( ", statePtr->WBEND.instr);
    printInstruction(statePtr->WBEND.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->WBEND.writeData);
    if (wbendOp >= SW || wbendOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    printf("end state\n");
}

// File
#define MAXLINELENGTH 1000 // MAXLINELENGTH is the max number of characters we read

void readMachineCode(stateType *state, char* filename) {
    char line[MAXLINELENGTH];
    FILE *filePtr = fopen(filename, "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", filename);
        exit(1);
    }

    printf("instruction memory:\n");
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state->numMemory) {
        if (sscanf(line, "%d", state->instrMem+state->numMemory) != 1) {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("\tinstrMem[ %d ] = ", state->numMemory);
        printInstruction(state->dataMem[state->numMemory] = state->instrMem[state->numMemory]);
        printf("\n");
    }
}