#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch(ALUControl) {
        // TODO: Ask if this should be parsed to binary
        case '0': // 000 add
            *ALUresult = (signed)A +(signed) B;
            break;
        case '1': // 001 subtract
            *ALUresult = (signed)A - (signed)B;
            break;
        case '2': // 010 set 1 if A < B else 0
            if ((signed)A < (signed)B)
                *ALUresult = 1;
            else
                *ALUresult = 0;
            break;
        case '3': // 011 set 1 if (unsigned)A < (unsigned)B else 0
            if (A < B)
                *ALUresult = 1;
            else
                *ALUresult = 0;
            break;
        case '4': // 100 A AND B
            *ALUresult = A && B;
            break;
        case '5': // 101 A OR B
            *ALUresult = A || B;
            break;
        case '6': // 110 shift left B 16 bits
            // TODO: Ask if this should be output to ALUresult
            B = B << 16;
            break;
        case '7': // 111 not
            *ALUresult = !A;
            break;
    }

    if (*ALUresult == 0)
        *Zero = '1';
    else
        *Zero = '0';
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // Check if PC is word aligned
    if (PC % 4 != 0)
        return 1;

    // Shift PC right by 2 to get proper index
    *instruction = Mem[PC >> 2];
    return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // Unless if the bits are already at the end of the string,
    // shift right to place the bits at the end and then bitmask
    // to isolate.
    *op = (instruction >> 26)   & 0b00000000000000000000000000111111;
    *r1 = (instruction >> 21)   & 0b00000000000000000000000000011111;
    *r2 = (instruction >> 16)   & 0b00000000000000000000000000011111;
    *r3 = (instruction >> 11)   & 0b00000000000000000000000000011111;
    *funct = instruction        & 0b00000000000000000000000000111111;
    *offset = instruction       & 0b00000000000000001111111111111111;
    *jsec = instruction         & 0b00000011111111111111111111111111;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
}

