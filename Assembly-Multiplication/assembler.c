/**
 * Project 1 
 * Assembler code fragment for LC-2K 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

// TEST 10 IS EITHER THE RANGE (EXIT1) OR THE UNRECOGNIZED OPCODE (EXIT1)

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);

// typedef struct {
//     char * label;
//     int address;
// } Label;

int calculateOffset(char *, char *, int, char labels[1000][7]);

int
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // First pass
    // Document all the labels and their corresponding locations
    char labels[1000][7];
    int i = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        // Check for duplicate labels
        for (int j = i; j > -1; j--) {
            if (label[0] != '\0' && strcmp(label, labels[j]) == 0) {
                printf("label: %s\n", label);
                printf("Duplicate definition of labels\n");
                exit(1);
            }
        }
        strcpy(labels[i], label);
        // printf("%s : %i %c", labels[i].label, labels[i].address, '\n');
        // printf("%i %s %c", i, label, '\n');
        ++i;
    }

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    // Second pass
    // Translate each line of the assembly file
    int PC = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        // printf("PC: %i\n", PC);
        int b_line = 0;

        // .fill
        if (strcmp(opcode, ".fill") == 0) {
            b_line += calculateOffset(arg0, opcode, PC, labels);
        }
    
        // R TYPE
        else if (strcmp(opcode, "add") == 0) {
            // b_line += 0 << 22; // 000
            b_line += atoi(arg0) << 19;
            b_line += atoi(arg1) << 16;
            b_line += atoi(arg2) << 0;
        }
        else if (strcmp(opcode, "nor") == 0) {
            b_line += 1 << 22; // 001
            b_line += atoi(arg0) << 19;
            b_line += atoi(arg1) << 16;
            b_line += atoi(arg2) << 0;
        }

        // I TYPE
        else if (strcmp(opcode, "lw") == 0) {
            b_line += 2 << 22; // 010
            b_line += atoi(arg0) << 19;
            b_line += atoi(arg1) << 16;
            b_line += calculateOffset(arg2, opcode, PC, labels);
            // printf("%i %c", b_line, '\n');
        }
        else if (strcmp(opcode, "sw") == 0) {
            b_line += 3 << 22; // 011
            b_line += atoi(arg0) << 19;
            b_line += atoi(arg1) << 16;
            b_line += calculateOffset(arg2, opcode, PC, labels);
        }
        else if (strcmp(opcode, "beq") == 0) {
            b_line += 4 << 22; // 100
            b_line += atoi(arg0) << 19;
            b_line += atoi(arg1) << 16;
            //printf("%s %s \n", arg0, arg1); // 0000000 100 000 001 000000000000...
            //printf("%i %c", b_line, '\n');
            b_line += calculateOffset(arg2, opcode, PC, labels);
            //printf("%i %c", b_line, '\n');
        }

        // J TYPE
        else if (strcmp(opcode, "jalr") == 0) {
            b_line += 5 << 22; // 101
            b_line += atoi(arg0) << 19;
            b_line += atoi(arg1) << 16;
        }

        // O TYPE
        else if (strcmp(opcode, "halt") == 0) {
            b_line += 6 << 22; // 110
        }
        else if (strcmp(opcode, "noop") == 0) {
            b_line += 7 << 22; // 111
        }

        // INVALID TYPE
        else {
            printf("unrecognized opcode: %s\n", opcode);
            exit(1);
        }
        PC = PC + 1;
        fprintf(outFilePtr, "%d\n", b_line);
        //printf("%i %c", b_line, '\n');
    }

    exit(0);

    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int calculateOffset(char *arg2, char *opcode, int PC, char labels[1000][7]) {
    int offset = atoi(arg2);
    // Check range
    if (strcmp(opcode, ".fill") != 0 && (offset > 32767 || offset < -32768)) {
        printf("offsetField doesn't fit in 16 bits");
        exit(1);
    }
    if (isNumber(arg2) != 1) { // If it is a label
        int found_label = 0;
        for (int i = 0; i < 1000; ++i) {
            if (strcmp(labels[i], arg2) == 0) {
                offset = i;
                found_label = 1;
                // printf("address %i %c", i, '\n');
            }
        }
        if (found_label == 0) {
            printf("Use of undefined label: %s!", arg2);
            exit(1);
        }

        // For beq, When arg2 is symbolic, we calculate offset field = address - 1 - PC
        if (strcmp(opcode, "beq") == 0) {
            offset = offset - 1 - PC;
            // if (offset < 0) { // if offset is negative
            //     offset = 65535 & offset;
            // }
            // // if offset is positive it is already formatted correctly
        }
    }

    // If offset is negative and opcode is beq, sw, or lw
    if (strcmp(opcode, "beq") == 0 || strcmp(opcode, "sw") == 0 || strcmp(opcode, "lw") == 0) {
        if (offset < 0) {
            // offset = 65535 & offset;
            offset = 0xffff & offset;
        }
    }

    return offset;
}
