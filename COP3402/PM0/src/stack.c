#include "pm0.h"

#define BASE(l, bp) (bp - l*6)

const int MAX_STACK_HEIGHT = 200;
const int MAX_CODE_LENGTH = 500;
const int MAX_LINE_LENGTH = 10;
const int MAX_LEXI_LEVEL = 3;
const char* WRITE = "w";
const char* READ = "r";

int stack(const char* inputPath, const char* outputPath) {

    // Open input file for reading
    FILE* inputPtr= openFile(inputPath, READ);
    if (inputPtr == NULL) {
        printf("[STACK-ERROR] Error opening \"%s\".\n", inputPath);
        return 1;
    }
    printf("[STACK-LOG] %s opened for input.\n", inputPath);

    // Open output file for writing
    FILE* outputPtr = openFile(outputPath, WRITE);
    if (outputPtr == NULL) {
        printf("Error opening \"%s\"\n", outputPath);
        return 1;
    }
    printf("[STACK-LOG] %s opened for output.\n", outputPath);

    // Intialize registers
    int SP = 0; // Points to the top of the stack
    int BP = 1; // Points to base of the activation record
    int PC = 0; // The index of the next instruction
    instruction* IR; // The current instruction container
    int halt = 0;
    printf("[STACK-LOG] Registers intialized.\n");

    // Initialize stack
    int stack[MAX_STACK_HEIGHT];
    memset(stack, 0, MAX_STACK_HEIGHT * sizeof(int));
    printf("[STACK-LOG] Stack of length %d initalized to 0.\n", MAX_STACK_HEIGHT);

    // Read instructions
    instruction instructions[MAX_CODE_LENGTH];
    read(inputPtr, instructions);
    printf("[STACK-LOG] Instructions loaded into memory, closing input file.\n");
    fclose(inputPtr);

    // Build and write Instructions string to file
    fprintf(outputPtr, "%s", buildInstructionsString(instructions));
    printf("[STACK-LOG] Instructions written to output file.\n");

    // Write header for stacktrace
    fprintf(outputPtr, "\t\t\t\tpc\tbp\tsp\tstack\n");
    fprintf(outputPtr, "Initial Values\t\t\t%d\t%d\t%d\n", PC, BP, SP);

    printf("[STACK-LOG] Begin fetch/execute cycle.\n");

    // Fetch Cycle
    while(halt == 0) {
        // Fetch instruction
        IR = &instructions[PC];
        printf("[STACK-LOG] FETCHED: %d %d %d\n", IR->opcode, IR->lex, IR->param);

        // Execute instruction
        int prevPC = PC;
        execute(IR, &PC, &SP, &BP, &halt, stack);
        printf("[STACK-LOG] EXECUTED: PC:%d SP:%d BP:%d HALT:%d\n", PC, SP, BP, halt);

        // Write execution trace line to file
        fprintf(outputPtr, "%s", buildTraceLine(prevPC, IR, PC, BP, SP, stack));
    }

    printf("[STACK-LOG] Stack operations halted, closing output file.\n");
    fclose(outputPtr);

    return 0;
}

/* Opens a file and returns a FILE pointer */
FILE* openFile(const char* path, const char* op) {
    FILE* filePtr;
    filePtr = fopen(path, op);
    if(filePtr == NULL) {
        perror("[STACK-ERROR] Error opening file.");
        return NULL;
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
    sprintf(text, "%s", "LINE\tOP\tL\tM\n");

    // Concat each instruction to a string
    int i = 0;
    while(i != -1) {
        sprintf(text+strlen(text), "%d\t%s\t%d\t%d\n",
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
    char* name = "";
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

    char* line = malloc(MAX_CODE_LENGTH);

    sprintf(line+strlen(line), "%d\t%s\t%d\t%d\t%d\t%d\t%d\t%s\n",
            prevPC,
            getOpcodeName(IR->opcode),
            IR->lex,
            IR->param,
            PC,
            BP,
            SP,
            stackString(stack, SP, BP));

    return line;
}

/* Build string showing the stack */
char* stackString(int* stack, int SP, int BP) {

    char* stackString = malloc(MAX_STACK_HEIGHT);

    int i = 1;
    // Loop through the stack from 1 to SP
    // When BP that is not 1 is encountered, print a divider
    while (i <= SP) {
        if (i == BP && BP != 1)
            sprintf(stackString+strlen(stackString), "| ");
        sprintf(stackString+strlen(stackString), "%d ", stack[i]);
        i++;
    }

    return stackString;
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
            *PC = *PC + 1;
            break;
        case 2: // OPR
            if (param == 0) {
                *SP = *BP - 1;
                *PC = stack[*SP + 4];
                *BP = stack[*SP + 3];
            } else {
                // TODO: Implement all OPR operations
                switch (param) {
                    case 1: // NEG
                        stack[*SP] = stack[*SP] * -1;
                        break;
                    case 2: // ADD
                    {
                        int sum = stack[*SP] + stack[*SP - 1];
                        *SP = *SP - 1;
                        stack[*SP] = sum;
                        break;
                    }
                    case 3: // SUB
                    {
                        int diff = stack[*SP - 1] - stack[*SP];
                        *SP = *SP - 1;
                        stack[*SP] = diff;
                        break;
                    }
                    case 4: // MUL
                    {
                        int product = stack[*SP] * stack[*SP - 1];
                        *SP = *SP - 1;
                        stack[*SP] = product;
                        break;
                    }
                    case 5: // DIV
                    {
                        int quo = stack[*SP - 1] / stack[*SP];
                        *SP = *SP - 1;
                        stack[*SP] = quo;
                        break;
                    }
                    case 6: // ODD
                        if (stack[*SP] % 2 == 1)
                            stack[*SP] = 1;
                        else
                            stack[*SP] = 0;
                        break;
                    case 7: // MOD
                    {
                        int mod = stack[*SP - 1] % stack[*SP];
                        *SP = *SP - 1;
                        stack[*SP] = mod;
                        break;
                    }
                    case 8: // EQL
                        if (stack[*SP] == stack[*SP - 1])
                            stack[*SP - 1] = 1;
                        else
                            stack[*SP - 1] = 0;
                        *SP = *SP - 1;
                        break;
                    case 9: // NEQ
                        if (stack[*SP] == stack[*SP - 1])
                            stack[*SP - 1] = 0;
                        else
                            stack[*SP - 1] = 1;
                        *SP = *SP - 1;
                        break;
                    case 10: // LSS
                        if (stack[*SP - 1] < stack[*SP])
                            stack[*SP - 1] = 1;
                        else
                            stack[*SP - 1] = 0;
                        *SP = *SP - 1;
                        break;
                    case 11: // LEQ
                        if (stack[*SP - 1] <= stack[*SP])
                            stack[*SP - 1] = 1;
                        else
                            stack[*SP - 1] = 0;
                        *SP = *SP - 1;
                        break;
                    case 12: // GTR
                        if (stack[*SP - 1] > stack[*SP])
                            stack[*SP - 1] = 1;
                        else
                            stack[*SP - 1] = 0;
                        *SP = *SP - 1;
                        break;
                    case 13: // GEQ
                        if (stack[*SP - 1] >= stack[*SP])
                            stack[*SP - 1] = 1;
                        else
                            stack[*SP - 1] = 0;
                        *SP = *SP - 1;
                        break;
                }
                *PC = *PC + 1;
            }
            break;
        case 3: // LOD
            *SP = *SP + 1;
            stack[*SP] = stack[BASE(lex, *BP) + param];
            *PC = *PC + 1;
            break;
        case 4: // STO
            stack[BASE(lex, *BP) + param] = stack[*SP];
            stack[*SP] = 0;
            *SP = *SP - 1;
            *PC = *PC + 1;
            break;
        case 5: // CAL
            stack[*SP + 1] = 0;
            stack[*SP + 2] = BASE(lex, *BP);
            stack[*SP + 3] = *BP;
            stack[*SP + 4] = *PC + 1;
            *BP = *SP + 1;
            *PC = param;
            break;
        case 6: // INC
            *SP = *SP + param;
            *PC = *PC + 1;
            break;
        case 7: // JMP
            *PC = param;
            break;
        case 8: // JPC
            if (stack[*SP] == 0)
                *PC = param;
            else
                *PC = *PC + 1;
            *SP = *SP - 1;
            break;
        case 9: // SIO 1
            printf("%d\n", stack[*SP]);
            *SP = *SP - 1;
            *PC = *PC + 1;
            break;
        case 10: // SIO 2
            *SP = *SP + 1;
            int result = scanf("%d", &stack[*SP]);
            if (result == 1)
                printf("[STACK-LOG] SIO 0 2 input read successfully.\n");
            else
                printf("[STACK-ERROR] SIO 0 2 error reading input.\n");
            *PC = *PC + 1;
            break;
        case 11: // SIO 3
            *halt = 1;
            *PC = 0;
            *SP = 0;
            *BP = 0;
            break;
    }
}
