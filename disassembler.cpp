#include <stdio.h>
#include <stdint.h>

#include "virtual-processor.h"

int main(int argc, char* argv[]) {

    if (argc != 3) {
        return 1;
    }

    return disassembler(argv[1], argv[2]);
}

const char* getCommandName(int opcode) {

    switch (opcode) {
        case MY_PUSH:   return "PUSH";
        case MY_POP:    return "POP";
        case MY_ADD:    return "ADD";
        case MY_SUB:    return "SUB";
        case MY_MUL:    return "MUL";
        case MY_DIV:    return "DIV";
        case MY_SQUARE: return "SQUARE";
        case MY_SIN:    return "SIN";
        case MY_COS:    return "COS";
        case MY_TG:     return "TG";
        case MY_CTG:    return "CTG";
        case MY_BREAK:  return "BREAK";
        case MY_IN:     return "IN";
        case MY_OUT:    return "OUT";
        case MY_HLT:    return "HLT";
        case MY_JMP:    return "JMP";
        case MY_CALL:   return "CALL";
        case MY_RET:    return "RET";
        case MY_JA:     return "JA";
        case MY_JB:     return "JB";
        case MY_JAE:    return "JAE";
        case MY_JBE:    return "JBE";
        case MY_JE:     return "JE";
        case MY_JNE:    return "JNE";
        case MY_JT:     return "JT";
        case MY_DRAW:   return "DRAW";
        default:        return NULL;
    }
}

const char* getRegisterName(int reg) {

    switch (reg) {
        case RAX: return "RAX";
        case RBX: return "RBX";
        case RCX: return "RCX";
        case RDX: return "RDX";
        default:  return NULL;
    }
}

void printArgument(FILE* output, const binaryInstruction* command) {

    bool hasMemory = (command->flags & ARG_MEM) != 0;
    bool hasReg    = (command->flags & ARG_REG) != 0;
    bool hasImm    = (command->flags & ARG_IMMED_VALUE) != 0;

    if (hasMemory)
        fprintf(output, " [");

    if (hasReg) {
        const char* regName = getRegisterName(command->reg);
        fprintf(output, "%s", regName ? regName : "UNKNOWN_REG");
    }

    if (hasImm) {
        if (hasReg && command->imm >= 0) fprintf(output, "+");
        fprintf(output, "%d", command->imm);
    }

    if (hasMemory)
        fprintf(output, "]");
}

int disassembler(const char* inputName, const char* outputName) {

    assert(inputName);
    assert(outputName);

    FILE* input = fopen(inputName, "rb");
    FILE* output = fopen(outputName, "w");

    binaryInstruction command = {};

    while (fread(&command, sizeof(command), 1, input) == 1) {

        const char* name = getCommandName(command.opcode);

        fprintf(output, "%s", name);

        if (command.opcode == MY_PUSH || command.opcode == MY_POP) {
            printArgument(output, &command);
        }
        else if (command.opcode == MY_JMP  ||
                 command.opcode == MY_CALL ||
                 command.opcode == MY_JA   ||
                 command.opcode == MY_JB   ||
                 command.opcode == MY_JAE  ||
                 command.opcode == MY_JBE  ||
                 command.opcode == MY_JE   ||
                 command.opcode == MY_JNE  ||
                 command.opcode == MY_JT) {

            fprintf(output, " %d", command.imm);
        }

        fprintf(output, "\n");
    }

    FCLOSE(input);
    FCLOSE(output);

    return 0;
}
