#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

#include "../universal-features/error.h"

struct disasmInstruction {
    int opcode;
    int flags;
    int reg;
    int imm;
};

bool parseDisasmLine(const char* line, disasmInstruction* out) {

    int n = 0;

    if (sscanf(line, " %d%n", &out->opcode, &n) != 1)
        return false;

    const char* rest = line + n;
    int consumed = 0;

    if (out->opcode == MY_PUSH || out->opcode == MY_POP) {

        if (sscanf(rest, " %d %d %d %n",
                   &out->flags, &out->reg, &out->imm,
                   &consumed) != 3)
            return false;
    }

    else if (out->opcode == MY_JMP || out->opcode == MY_CALL ||
             out->opcode == MY_JA  || out->opcode == MY_JB   ||
             out->opcode == MY_JAE || out->opcode == MY_JBE  ||
             out->opcode == MY_JE  || out->opcode == MY_JNE  ||
             out->opcode == MY_JT) {

        if (sscanf(rest, " %d %n", &out->imm, &consumed) != 1)
            return false;
    }

    else {
        sscanf(rest, " %n", &consumed);
    }

    rest += consumed;

    while (*rest == ' ' || *rest == '\t' ||
           *rest == '\r' || *rest == '\n')
        rest++;

    return *rest == '\0';
}

// Название команды по её числовому коду
const char* disasmSimpleName(int command) {

    switch (command) {

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

        default:        return NULL;
    }
}

// Проверка команды перехода
static bool disasmIsJump(int command) {

    return command == MY_JMP ||
           command == MY_CALL ||
           command == MY_JA ||
           command == MY_JB ||
           command == MY_JAE ||
           command == MY_JBE ||
           command == MY_JE ||
           command == MY_JNE ||
           command == MY_JT;
}

ErrorCode disassembler(files* usedFiles) {
    if (usedFiles == NULL)
        return ERR_INVALID_ARGUMENT;

    FILE* exeFile = fopen(usedFiles->exeFile, "r");

    if (exeFile == NULL)
        return ERR_FILE_NOT_FOUND;

    std::vector<disasmInstruction> instructions;

    char line[MAX_COMMAND_LENGTH] = {};

    // Первый проход: чтение команд
    while (fgets(line, sizeof(line), exeFile) != NULL) {

        disasmInstruction instruction = {};

        if (!parseDisasmLine(line, &instruction)) {
            FCLOSE(exeFile);
            return ERR_INVALID_DATA;
        }

        instructions.push_back(instruction);
    }

    if (ferror(exeFile)) {
        FCLOSE(exeFile);
        return ERR_IO;
    }

    FCLOSE(exeFile);

    const char* registerNames[REGISTER_COUNT] = {
        "RAX", "RBX", "RCX", "RDX"
    };

    std::vector<bool> labels(instructions.size(), false);

    // Второй проход: поиск меток и проверка команд
    for (size_t i = 0; i < instructions.size(); i++) {

        const disasmInstruction& instruction = instructions[i];

        if (disasmIsJump(instruction.opcode)) {

            int address = instruction.imm;

            if (address < 0 ||
                (size_t)address >= instructions.size())
                return ERR_OUT_OF_BOUNDS;

            labels[address] = true;
        }

        if (instruction.opcode == MY_PUSH ||
            instruction.opcode == MY_POP) {

            if (instruction.reg < 0 ||
                instruction.reg >= REGISTER_COUNT)
                return ERR_INVALID_DATA;

            if (instruction.opcode == MY_POP) {

                if (instruction.flags != ARG_REG &&
                    instruction.flags != (ARG_REG | ARG_MEM))
                    return ERR_INVALID_DATA;

                if (instruction.imm != 0)
                    return ERR_INVALID_DATA;
            }

            if (instruction.opcode == MY_PUSH) {

                if (instruction.flags < 1 ||
                    instruction.flags > 7 ||
                    instruction.flags == ARG_MEM)
                    return ERR_INVALID_DATA;
            }
        }

        else if (disasmSimpleName(instruction.opcode) == NULL) {
            return ERR_INVALID_DATA;
        }
    }

    FILE* asmFile = fopen(usedFiles->asmProgramFile, "w");

    if (asmFile == NULL)
        return ERR_FILE_NOT_FOUND;

    // Третий проход: восстановление ассемблерного кода
    for (size_t i = 0; i < instructions.size(); i++) {

        const disasmInstruction& instruction = instructions[i];

        if (labels[i])
            fprintf(asmFile, ":label_%u\n", (unsigned)i);

        if (instruction.opcode == MY_PUSH ||
            instruction.opcode == MY_POP) {

            const char* commandName =
                instruction.opcode == MY_PUSH ? "PUSH" : "POP";

            bool hasMemory =
                (instruction.flags & ARG_MEM) != 0;

            bool hasRegister =
                (instruction.flags & ARG_REG) != 0;

            bool hasImmediate =
                (instruction.flags & ARG_IMMED) != 0;

            fprintf(asmFile, "%s ", commandName);

            if (hasMemory)
                fprintf(asmFile, "[");

            if (hasRegister)
                fprintf(asmFile, "%s",
                        registerNames[instruction.reg]);

            if (hasImmediate) {

                if (hasRegister)
                    fprintf(asmFile, "+%d", instruction.imm);
                else
                    fprintf(asmFile, "%d", instruction.imm);
            }

            if (hasMemory)
                fprintf(asmFile, "]");

            fprintf(asmFile, "\n");
        }

        else if (disasmIsJump(instruction.opcode)) {

            fprintf(asmFile, "%s :label_%d\n",
                    disasmSimpleName(instruction.opcode),
                    instruction.imm);
        }

        else {

            fprintf(asmFile, "%s\n",
                    disasmSimpleName(instruction.opcode));
        }
    }

    if (fclose(asmFile) != 0)
        return ERR_IO;

    return ERR_OK;
}
