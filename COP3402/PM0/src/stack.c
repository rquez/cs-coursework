#include "pm0.h"

const int MAX_STACK_HEIGHT = 200;
const int MAX_CODE_LENGTH = 500;
const int MAX_LINE_LENGTH = 10;
const int MAX_LEXI_LEVEL = 3;
const char* WRITE = "w";
const char* READ = "r";

void stack(const char* inputPath, const char* outputPath) {

    // Open input file for reading
    FILE* inputPtr= openFile(inputPath, READ);
    printf("[LOG] %s opened for input.\n", inputPath);

    // Open output file for writing
    FILE* outputPtr = openFile(outputPath, WRITE);
    printf("[LOG] %s opened for output.\n", outputPath);

    printf("[LOG] Start stack operations\n");

    int SP = 0;
    int BP = 0;
    int PC = 0;
    instruction* IR;
    int halt = 0;

    printf("[LOG] Registers intialized\n");

    // Initialize stack
    int stack[MAX_STACK_HEIGHT];
    memset(stack, 0, sizeof(int));
    printf("[LOG] Stack of length %d initalized to 0.\n", MAX_STACK_HEIGHT);

    // Read instructions
    instruction instructions[MAX_CODE_LENGTH];
    read(inputPtr, instructions);
    printf("[LOG] Instructions loaded, closing input file.\n");
    fclose(inputPtr);

    // Build and write Instructions string to file
    fprintf(outputPtr, "%s", buildInstructionsString(instructions));
    printf("[LOG] Instructions written to output file.\n");

    // Fetch Cycle
    while(halt == 0) {
        // Fetch instruction
        IR = &instructions[PC];
        printf("[LOG] FETCHED: %d %d %d\n", IR->opcode, IR->lex, IR->param);

        // Execute instruction and return new PC
        int prevPC = PC;
        execute(IR, &PC, &SP, &BP, &halt, stack);
        printf("[LOG] EXECUTED: PC:%d SP:%d BP:%d HALT:%d\n", PC, SP, BP, halt);

        // Write execution trace line to file
        //fprintf(outputPtr, "%s", buildTraceLine(prevPC, IR, PC, BP, SP, stack));
    }

    printf("[LOG] Stack operations halted, closing output file..\n");
    fclose(outputPtr);
}

/* Opens a file and returns a FILE pointer */
FILE* openFile(const char* path, const char* op) {
    FILE* filePtr;
    filePtr = fopen(path, op);
    if(filePtr == NULL) {
        perror("Error opening input file.");
    }
    return filePtr;
}

/* Fills the instruction array with instructions from the input file */
void read(FILE* inputPtr, instruction* instructions) {

    // Char array of each line in the file
    char line[MAX_LINE_LENGTH];

    // Loop through the end of the file
    int i = 0;
    while ( fgets(line, MAX_LINE_LENGTH, (FILE*)inputPtr) != NULL ) {
        // Parse token chars to int and assign instruction
        instructions[i].opcode = (int)atoi((char *)strtok(line, " "));
        instructions[i].lex = (int)atoi((char *)strtok(NULL, " "));
        instructions[i].param = (int)atoi((char *)strtok(NULL, " "));
        i++;
    }
}

/* Build string showing all instructions */
char* buildInstructionsString(instruction* instructions) {
    char* text = malloc(MAX_CODE_LENGTH);

    // Header
    sprintf(text, "%s", "LINE\tOP\tL\tM\n ");

    // Concat each instruction to a string
    int i = 0;
    while(i != -1) {
        sprintf(text+strlen(text), "%d\t%s\t%d\t%d\n ",
                i,
                getOpcodeName(instructions[i].opcode),
                instructions[i].lex,
                instructions[i].param);
        // Check if last instruction is reached
        if (instructions[i].opcode == 11 && instructions[i].param == 3)
            i = -1;
        else
            i++;
    }
    return text;
}

/* Returns the 3 letter name of the opcode */
char* getOpcodeName(int opcode) {
    char* name;
    switch (opcode) {
        case 1: // LIT
            name = "lit";
            break;
        case 2: // OPR
            name = "opr";
            break;
        case 3: // LOD
            name = "lod";
            break;
        case 4: // STO
            name = "sto";
            break;
        case 5: // CAL
            name = "cal";
            break;
        case 6: // INC
            name = "inc";
            break;
        case 7: // JMP
            name = "jmp";
            break;
        case 8: // JPC
            name = "jpc";
            break;
        case 9: // SIO
        case 10:
        case 11:
            name = "sio";
            break;
    }
    return name;
}

/* Build string showing execution trace line */
char* buildTraceLine(int prevPC, instruction* IR, int PC, int BP, int SP, int* stack) {

}

/* Executes the instruction IR and increments the PC */
void execute(instruction* IR, int* PC, int* SP, int* BP, int* halt, int* stack) {

    // Read Instructions
    int opcode = IR->opcode;
    int lex = IR->lex;
    int param =  IR->param;

    // Switch statement on opcode
    switch (opcode) {
        case 1: // LIT
            *SP = *SP + 1;
            stack[*SP] = param;
            break;
        case 2: // OPR
            // TODO: Implement OPR
            break;
        case 3: // LOD
            *SP = *SP + 1;
            stack[*SP] = stack[lex + param];
            break;
        case 4: // STO
            stack[lex + param] = stack[*SP];
            *SP = *SP - 1;
            break;
        case 5: // CAL
            // TODO: Implement CAL
            break;
        case 6: // INC
            *SP = *SP + param;
            break;
        case 7: // JMP
            *PC = param;
            return;
        case 8: // JPC
            // TODO: Implement JPC
            break;
        case 9: // SIO 1
            printf("%d\n", stack[*SP]);
            *SP = *SP - 1;
            break;
        case 10: // SIO 2
            break;
        case 11: // SIO 3
            *halt = 1;
            break;
    }

    // Increment PC
    *PC = *PC + 1;
}
