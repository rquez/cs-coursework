#include "parser.h"

void parse(const char* symbolTablePath, const char* mcodePath, token* tokens, symbol* symbolTablePtr, instruction* instructions) {

    program(tokens, symbolTablePtr, 0);
    FILE* symTblPtr = openFileParser(symbolTablePath, "w");
    writeSymbolTable(symbolTablePtr, symTblPtr);
    FILE* mcodePtr = openFileParser(symbolTablePath, "w");
}

/* Opens a file and returns a FILE pointer */
FILE* openFileParser(const char* path, const char* op) {
    FILE* filePtr;
    filePtr = fopen(path, op);
    if(filePtr == NULL) {
        fprintf(stderr, "[PARSER-ERROR] Error opening %s\n", path);
        perror("");
        return NULL;
    }
    return filePtr;
}

/* Writes symbol table to file */
void writeSymbolTable(symbol* symbolTable, FILE* symTblPtr) {
    int i = 0;
    fprintf(symTblPtr, "Name\tType\tValue\tLevel\n");
    for (i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++) {
        if (strcmp(symbolTable[i].name, "*") != 0) {
            switch (symbolTable[i].kind) {
                case 1:
                    fprintf(symTblPtr, "%s\tconst\t%d\t%d\n", symbolTable[i].name, symbolTable[i].level, symbolTable[i].value);
                    break;
                case 2:
                    fprintf(symTblPtr, "%s\tvar\t%d\t%d\n", symbolTable[i].name, symbolTable[i].level, symbolTable[i].value);
                    break;
                case 3:
                    fprintf(symTblPtr, "%s\tproc\t%d\t%d\n", symbolTable[i].name, symbolTable[i].level, symbolTable[i].value);
                    break;
            }
        }
    }
}

/* Adds token to symbol table */
void addToSymbolTable(symbol** symbolTable, char* name, int kind, int value, int level, int address) {
    int index = hashToken(name, kind) % MAX_SYMBOL_TABLE_SIZE;

    // Detect collision
    if ((*symbolTable)[index].kind != 0) {
        linearProbe(&index, symbolTable);
    }

    strcpy((*symbolTable)[index].name,name);
    (*symbolTable)[index].kind = kind;
    (*symbolTable)[index].value = value;
    (*symbolTable)[index].level = level;
    (*symbolTable)[index].address = address;
}

/* Adds the instruction to the instructions array and increments the index */
void emit(int opcode, int level, int modifier, int* cx, instruction** instructions) {

    if (*cx > MAX_SYMBOL_TABLE_SIZE) {
        fprintf(stderr, "[PARSER-ERROR] Can't have more than %d instructions.\n", MAX_SYMBOL_TABLE_SIZE);
    }

    (*instructions)[*cx].opcode = opcode;
    (*instructions)[*cx].level = level;
    (*instructions)[*cx].modifier = modifier;
    (*cx)++;
}

/* Hash function that hashs on name, kind and a prime */
int hashToken(char* name, int kind) {
    int sum = 0;
    for (; *name; name++) {
        sum += *name;
    }
    return (sum + kind) * 71;
}

/* Returns 1 if ident is a const
 * Returns 2 if ident is a var
 * Returns 3 if ident is a proc
 * Returns 0 if ident is not in symbol table
 */
int lookupIdentifier(char* name, symbol** symbolTable, int level) {

    int indexCon = hashToken(name, 1) % MAX_SYMBOL_TABLE_SIZE;
    int indexVar = hashToken(name, 2) % MAX_SYMBOL_TABLE_SIZE;
    int indexPro = hashToken(name, 3) % MAX_SYMBOL_TABLE_SIZE;

    if (strcmp((*symbolTable)[indexCon].name, name) == 0 && (*symbolTable)[indexCon].level <= level)
        return 1;
    if (strcmp((*symbolTable)[indexVar].name, name) == 0 && (*symbolTable)[indexVar].level <= level)
        return 2;
    if (strcmp((*symbolTable)[indexPro].name, name) == 0 && (*symbolTable)[indexPro].level <= level)
        return 3;

    return 0;
}

/* Linear probing collision resolution */
void linearProbe(int* index, symbol** symbolTable) {
    // symbol's kind field will either be 1, 2 or 3
    while ((*symbolTable)[(*index) % MAX_SYMBOL_TABLE_SIZE].kind != 0) {
        *index += 1;
    }
}

/* program := block periodsym */
void program(token* tokens, symbol* symbolTable, int level) {
    block(&tokens, symbolTable, level);
    if (tokens->type != periodsym) {
        fprintf(stderr, "[PARSER-ERROR] '.' expected. line %d\n", tokens->lineNumber);
        exit(EXIT_FAILURE);
    }
}

/* block := [constant] [variable] {procedure} [statement] */
void block(token** tokens, symbol* symbolTable, int level) {

    if ((*tokens)->type == constsym) {
        constant(tokens, symbolTable, level);
    }

    if ((*tokens)->type == varsym) {
        variable(tokens, symbolTable, level);
    }

    while ((*tokens)->type == procsym) {
        procedure(tokens, symbolTable, level);
    }

    statement(tokens, symbolTable, level);

    if ((*tokens)->type != semicolonsym && (*tokens)->type != periodsym) {
        fprintf(stderr, "[PARSER-ERROR] Incorrect symbol after statement part in block. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }
}

/* constant := constsym identsym equalsym numbersym {commasym identsym equalsym numbersym} semicolonsym*/
void constant(token** tokens, symbol* symbolTable, int level) {

    do {
        *tokens = (*tokens)->next;
        if ((*tokens)->type != identsym) {
            fprintf(stderr, "[PARSER-ERROR] 'const' must be followed by an identifier. line %d\n", (*tokens)->lineNumber);
            exit(EXIT_FAILURE);
        }
        char* lexeme = (*tokens)->lexeme;

        *tokens = (*tokens)->next;
        if ((*tokens)->type != equalsym) {
            if ((*tokens)->type == becomesym) {
                fprintf(stderr, "[PARSER-ERROR] Use = instead of ':='. line %d\n", (*tokens)->lineNumber);
            } else {
                fprintf(stderr, "[PARSER-ERROR] '=' expected. line %d\n", (*tokens)->lineNumber);
            }
            exit(EXIT_FAILURE);
        }

        *tokens = (*tokens)->next;
        if ((*tokens)->type != numbersym) {
            fprintf(stderr, "[PARSER-ERROR] '=' must be followed by a number. line %d\n", (*tokens)->lineNumber);
            exit(EXIT_FAILURE);
        }

        addToSymbolTable(&symbolTable, lexeme, 1, (*tokens)->value, level, 0);

        *tokens = (*tokens)->next;

    } while((*tokens)->type == commasym);

    if ((*tokens)->type != semicolonsym) {
        fprintf(stderr, "[PARSER-ERROR] ';' missing. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }

    *tokens = (*tokens)->next;
}

/* variable := varsym identsym {commasym identsym} semicolonsym */
void variable(token** tokens, symbol* symbolTable, int level) {

    do {
        *tokens = (*tokens)->next;
        if ((*tokens)->type != identsym) {
            fprintf(stderr, "[PARSER-ERROR] 'var' must be followed by an identifier. line %d\n", (*tokens)->lineNumber);
            exit(EXIT_FAILURE);
        }

        addToSymbolTable(&symbolTable, (*tokens)->lexeme, 2, 0, level, 0);

        *tokens = (*tokens)->next;
    } while ((*tokens)->type == commasym);

    if ((*tokens)->type != semicolonsym) {
        fprintf(stderr, "[PARSER-ERROR] ';' missing. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }

    *tokens = (*tokens)->next;
}

/* procedure := procsym identsym semicolonsym block semicolonsym */
void procedure(token** tokens, symbol* symbolTable, int level) {

    *tokens = (*tokens)->next;
    if ((*tokens)->type != identsym) {
        fprintf(stderr, "[PARSER-ERROR] 'procedure' must be followed by an identifier. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }

    addToSymbolTable(&symbolTable, (*tokens)->lexeme, 3, 0, level, 0);

    *tokens = (*tokens)->next;
    if ((*tokens)->type != semicolonsym) {
        fprintf(stderr, "[PARSER-ERROR] ';' missing. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }

    *tokens = (*tokens)->next;
    if (
            (*tokens)->type != constsym &&
            (*tokens)->type != varsym &&
            (*tokens)->type != procsym &&
            (*tokens)->type != identsym &&
            (*tokens)->type != callsym &&
            (*tokens)->type != beginsym &&
            (*tokens)->type != ifsym &&
            (*tokens)->type != whilesym &&
            (*tokens)->type != readsym &&
            (*tokens)->type != writesym) {

        fprintf(stderr, "[PARSER-ERROR] Incorrect symbol after procedure declaration. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }
    block(tokens, symbolTable, ++level);

    if ((*tokens)->type != semicolonsym) {
        fprintf(stderr, "[PARSER-ERROR] ';' missing. line %d\n", (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }

    *tokens = (*tokens)->next;
}

/*
 * statement :=     identsym becomesym expression
 *              |   callsym identsym
 *              |   beginsym statement {semicolonsym statement} endsym
 *              |   ifsym condition thensym statement [elsesym statement]
 *              |   whilesym condition dosym statement
 *              |   readsym identsym
 *              |   writesym identsym
 *              |   e
 */
void statement(token** tokens, symbol* symbolTable, int level) {

    switch((*tokens)->type) {

        // ident becomesym expression
        case identsym:

            // This identifier can only be a variable
            switch(lookupIdentifier((*tokens)->lexeme, &symbolTable, level)) {
                case 0:
                    fprintf(stderr, "[PARSER-ERROR] '%s' Undeclared identifier. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
                case 1:
                    fprintf(stderr, "[PARSER-ERROR] '%s' is a constant, assignment is not allowed. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
                case 3:
                    fprintf(stderr, "[PARSER-ERROR] '%s' is a procedure, assignment is not allowed. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
            }

            *tokens = (*tokens)->next;
            if ((*tokens)->type != becomesym) {
                fprintf(stderr, "[PARSER-ERROR] Assignment operator expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }

            *tokens = (*tokens)->next;
            expression(tokens, symbolTable, level);

            if ((*tokens)->type != semicolonsym) {
                fprintf(stderr, "[PARSER-ERROR] ';' missing. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }

            break;

        // callsym identsym
        case callsym:

            *tokens = (*tokens)->next;
            if ((*tokens)->type != identsym) {
                fprintf(stderr, "[PARSER-ERROR] Identifier must be followed by 'call'. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }

            // This identifier can only be a variable
            switch(lookupIdentifier((*tokens)->lexeme, &symbolTable, level)) {
                case 0:
                    fprintf(stderr, "[PARSER-ERROR] '%s' Undeclared identifier. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
                case 1:
                    fprintf(stderr, "[PARSER-ERROR] Call of a constant is meaningless. line %d\n", (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
                case 2:
                    fprintf(stderr, "[PARSER-ERROR] Call of a variable is meaningless. line %d\n", (*tokens)->lineNumber) ;
                    exit(EXIT_FAILURE);
            }

            *tokens = (*tokens)->next;

            break;

        // beginsym statement {semicolonsym statement} endsym
        case beginsym:
            *tokens = (*tokens)->next;

            if ((*tokens)->type != identsym &&
                    (*tokens)->type != callsym &&
                    (*tokens)->type != beginsym &&
                    (*tokens)->type != ifsym &&
                    (*tokens)->type != whilesym &&
                    (*tokens)->type != readsym &&
                    (*tokens)->type != writesym) {

                fprintf(stderr, "[PARSER-ERROR] Statement expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            statement(tokens, symbolTable, level);

            while ((*tokens)->type == semicolonsym) {
                *tokens = (*tokens)->next;
                statement(tokens,symbolTable, level);
            }

            if ((*tokens)->type != endsym) {
                fprintf(stderr, "[PARSER-ERROR] 'end' expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            *tokens = (*tokens)->next;
            break;

        // ifsym condition thensym statement [elsesym statement]
        case ifsym:
            *tokens = (*tokens)->next;

            condition(tokens, symbolTable, level);

            if ((*tokens)->type != thensym) {
                fprintf(stderr, "[PARSER-ERROR] 'then' expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            *tokens = (*tokens)->next;

            if ((*tokens)->type != identsym &&
                    (*tokens)->type != callsym &&
                    (*tokens)->type != beginsym &&
                    (*tokens)->type != ifsym &&
                    (*tokens)->type != whilesym &&
                    (*tokens)->type != readsym &&
                    (*tokens)->type != writesym) {

                fprintf(stderr, "[PARSER-ERROR] Statement expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            statement(tokens, symbolTable, level);

            if ((*tokens)->type == elsesym) {
                *tokens = (*tokens)->next;
                statement(tokens, symbolTable, level);
            }
            break;

        // whilesym condition dosym statement
        case whilesym:
            *tokens = (*tokens)->next;

            condition(tokens, symbolTable, level);

            if ((*tokens)->type != dosym) {
                fprintf(stderr, "[PARSER-ERROR] 'do' expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            *tokens = (*tokens)->next;

            if ((*tokens)->type != identsym &&
                    (*tokens)->type != callsym &&
                    (*tokens)->type != beginsym &&
                    (*tokens)->type != ifsym &&
                    (*tokens)->type != whilesym &&
                    (*tokens)->type != readsym &&
                    (*tokens)->type != writesym) {

                fprintf(stderr, "[PARSER-ERROR] Statement expected. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            statement(tokens, symbolTable, level);

            break;

        // readsym identsym
        case readsym:
            *tokens = (*tokens)->next;
            if ((*tokens)->type != identsym) {
                fprintf(stderr, "[PARSER-ERROR] An identifier must follow 'read'. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            *tokens = (*tokens)->next;
            break;

        // writesym identsym
        case writesym:
            *tokens = (*tokens)->next;
            if ((*tokens)->type != identsym) {
                fprintf(stderr, "[PARSER-ERROR] An identifier must follow 'write'. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            *tokens = (*tokens)->next;
            break;

        // empty string case
        default:
            break;

    }
}

/* condition :=     oddsym expression
 *              |   expression rel-op expression
 */
void condition(token** tokens, symbol* symbolTable, int level) {

    if ((*tokens)->type == oddsym) {

        (*tokens) = (*tokens)->next;
        expression(tokens, symbolTable, level);

    } else {

        expression(tokens, symbolTable, level);

        // relational operations between [9-14]
        if ((*tokens)->type < 9 || (*tokens)->type > 14) {
            fprintf(stderr, "[PARSER-ERROR] relational operator expected. line %d\n", (*tokens)->lineNumber);
            exit(EXIT_FAILURE);
        }
        (*tokens) = (*tokens)->next;

        expression(tokens, symbolTable, level);
    }
}

/* expression := [plussym | minussym] term { (plussym | minussym) term } */
void expression(token** tokens, symbol* symbolTable, int level) {

    if ((*tokens)->type == plussym || (*tokens)->type == minussym) {
        (*tokens) = (*tokens)->next;
    }

    if ((*tokens)->type != identsym && (*tokens)->type != numbersym && (*tokens)->type != lparentsym) {
        fprintf(stderr, "[PARSER-ERROR] An expression cannot begin with '%s'. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
        exit(EXIT_FAILURE);
    }

    term(tokens, symbolTable, level);

    while ((*tokens)->type == plussym || (*tokens)->type == minussym) {
        (*tokens) = (*tokens)->next;
        term(tokens, symbolTable, level);
    }
}

/* term := factor { (multsym | slashsym) factor } */
void term(token** tokens, symbol* symbolTable, int level) {

    factor(tokens, symbolTable, level);

    while ((*tokens)->type == multsym || (*tokens)->type == slashsym) {
        (*tokens) = (*tokens)->next;
        factor(tokens, symbolTable, level);
    }
}

/* factor := identsym | numbersym | lparentsym expression rparentsym */
void factor(token** tokens, symbol* symbolTable, int level) {

    switch((*tokens)->type) {

        case identsym:
            // This identifier can only be a variable or constant
            switch(lookupIdentifier((*tokens)->lexeme, &symbolTable, level)) {
                case 0:
                    fprintf(stderr, "[PARSER-ERROR] '%s' Undeclared identifier. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
                case 3:
                    fprintf(stderr, "[PARSER-ERROR] '%s' is a procedure and can't be used in an expression. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
                    exit(EXIT_FAILURE);
            }

            (*tokens) = (*tokens)->next;
            break;

        case numbersym:
            (*tokens) = (*tokens)->next;
            break;

        case lparentsym:
            (*tokens) = (*tokens)->next;
            expression(tokens, symbolTable, level);
            if ((*tokens)->type != rparentsym) {
                fprintf(stderr, "[PARSER-ERROR] ')' is missing. line %d\n", (*tokens)->lineNumber);
                exit(EXIT_FAILURE);
            }
            (*tokens) = (*tokens)->next;
            break;

        default:
            fprintf(stderr, "[PARSER-ERROR] The preceding factor cannot begin with '%s'. line %d\n", (*tokens)->lexeme, (*tokens)->lineNumber);
            exit(EXIT_FAILURE);
    }
}
