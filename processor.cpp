#include <TXLib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../universal-features/error.h"
#include "../universal-features/colors.h"
#include "../Task5 stack/stack.cpp"
#include "virtual-processor.h"
#include "math-functions.cpp"
#include "extra-functions.cpp"

#define BG_RED "\033[41m"

int main(int argc, char* argv[]){

    cancelBuffering();

    // Processor

    char exeFile[MAX_PATH_LENGTH] = {};
    ErrorCode error = setBinFilename(exeFile, MAX_PATH_LENGTH, argc, argv);
    if(error) return error;

    CPUInfo* memory = (CPUInfo*)calloc(1, sizeof(CPUInfo));
    if (memory == NULL) {
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Not enough memory", ERR_OUT_OF_MEMORY);
        printErrorIntoConsole(&debugLogInfo);
        return ERR_OUT_OF_MEMORY;
    }

    struct CPUInfo* CPUData = memory;
    struct stack_t stk = {};

    error = CPULoad(CPUData, &stk, exeFile);
    if (error != ERR_OK) return error;

    #ifdef CPU_DEBUG
        CPUData->debugInfo = {
            "CPUData",
            __FILE__,
            __func__,
            __LINE__
        };
    #endif

    error = CPUexecute(CPUData);

    CPUDestroy(CPUData);


    return error;
}

ErrorCode CPULoad(CPUInfo* CPUData, stack_t* stk, char* exeProgramFile){

    assert(CPUData);
    assert(stk);
    assert(exeProgramFile);

    ErrorCode error = readBinaryProgram(exeProgramFile, CPUData);
    if (error != ERR_OK) return error;

    error = STACK_INIT(stk, 3);
    if(error){
        printf("The function stackInit was completed with an error code: %d\n", error);
        return error;
    }

    CPUData->stack = stk;

    error = STACK_INIT(&CPUData->returnAddresses, 3);
    if (error != ERR_OK) {
        printf("The function stackInit was completed with an error code: %d\n", error);
        return error;
    }

    CPUData->IP = 0;

    return ERR_OK;
}

ErrorCode readBinaryProgram(const char* fileName, CPUInfo* CPUData) {

    assert(fileName);
    assert(CPUData);

    FILE* file = fopen(fileName, "rb");

    if (file == NULL) {
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "File-problem", ERR_FILE_NOT_FOUND);
        printErrorIntoConsole(&debugLogInfo);
        printf("Cannot open binary file: %s\n", fileName);

        return ERR_FILE_NOT_FOUND;
    }

    struct stat fileInfo = {};
    if (stat(fileName, &fileInfo) != 0) {
    printf("ERROR: Cannot get file size: %s\n", fileName);
    FCLOSE(file);
    return ERR_OK;
}
    long fileSize = fileInfo.st_size;

    if (fileSize <= 0) {

        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "", ERR_INVALID_FORMAT);
        printErrorIntoConsole(&debugLogInfo);
        printf("Invalid binary file size: %ld\n", fileSize);

        FCLOSE(file);
        return ERR_INVALID_FORMAT;
    }

    size_t commandsCount = (size_t)fileSize / sizeof(binaryInstruction);

    binaryInstruction* memory = (binaryInstruction*)calloc(commandsCount, sizeof(binaryInstruction));

    if (memory == NULL) {
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "OUT_OF_MEMORY", ERR_OUT_OF_MEMORY);
        printErrorIntoConsole(&debugLogInfo);

        FCLOSE(file);
        return ERR_OUT_OF_MEMORY;
    }

    fread(memory, sizeof(binaryInstruction), commandsCount, file);

    FCLOSE(file);

    CPUData->program = memory;
    CPUData->stringsCount = commandsCount;


    return ERR_OK;
}

ErrorCode CPUexecute(CPUInfo* CPUData){

    while (CPUData->IP < CPUData->stringsCount) {

        binaryInstruction* instruction = &CPUData->program[CPUData->IP];

        int command = instruction->opcode;
        int flags   = instruction->flags;
        int reg     = instruction->reg;
        int imm     = instruction->imm;

        switch(command){

            case MY_PUSH: {

                int res = 0;

                if (flags & ARG_REG) {

                    if (reg < 0 || reg >= REGISTER_COUNT){
                            error_debug debugLogInfo = {};
                            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Incorrect register", ERR_INVALID_ARGUMENT);
                            debugLogInfo.errCode = ERR_INVALID_ARGUMENT;
                            printErrorIntoConsole(&debugLogInfo);

                            return ERR_INVALID_ARGUMENT;
                    }


                    res += CPUData->registers[reg];
                }

                if (flags & ARG_IMMED_VALUE) res += imm;

                if (flags & ARG_MEM) {

                    int address = res;

                    res = CPUData->RAM[address];
                }

                ErrorCode error = stackPush(CPUData->stack, res * 1000);
                if (error != ERR_OK) return error;

                break;
            }

            case MY_POP: {

                if (flags != ARG_REG &&
                    flags != (ARG_MEM | ARG_REG) &&
                    flags != (ARG_MEM | ARG_IMMED_VALUE) &&
                    flags != (ARG_MEM | ARG_REG | ARG_IMMED_VALUE)) {

                    return ERR_INVALID_ARGUMENT;
                }

                if ((flags & ARG_REG) && (reg < 0 || reg >= REGISTER_COUNT)) return ERR_INVALID_ARGUMENT;

                int address = 0;

                if (flags & ARG_MEM) {

                    if (flags & ARG_REG) address += CPUData->registers[reg];

                    if (flags & ARG_IMMED_VALUE) address += imm;

                    if (address < 0 || address >= (int)RAM_CAPACITY) return ERR_OUT_OF_BOUNDS;
                }

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) return ERR_INVALID_DATA;

                int res = 0;

                ErrorCode error = stackPop(CPUData->stack, &res);
                if (error != ERR_OK) return error;

                if (flags & ARG_MEM) {
                    CPUData->RAM[address] = (int)(res / 1000);
                }
                else {
                    CPUData->registers[reg] = (int)(res / 1000);
                }

                break;
            }

            case MY_ADD:{

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                addCommand(CPUData->stack);

                break;
            }

            case MY_SUB: {

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                subCommand(CPUData->stack);

                break;
            }

            case MY_MUL:{

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                mulCommand(CPUData->stack);

                break;
            }

            case MY_DIV: {

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                divCommand(CPUData->stack);

                break;
            }

            case MY_SQUARE: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");

                    return ERR_INVALID_DATA;
                }

                squareCommand(CPUData->stack);

                break;
            }

            case MY_SIN:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                sinCommand(CPUData->stack);

                break;
            }

            case MY_COS:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                cosCommand(CPUData->stack);

                break;
            }

            case MY_TG: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                tgCommand(CPUData->stack);

                break;
            }

            case MY_CTG: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                ctgCommand(CPUData->stack);

                break;
            }

            case MY_BREAK:{
                *getRightDataCanary(CPUData->stack) = 8;
                break;
            }

            case MY_IN:{

                consoleInput(CPUData);

                break;
            }

            case MY_OUT:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    return ERR_INVALID_DATA;
                }

                stackElem_t value = 0;
                stackPop(CPUData->stack, &value);
                printf(COLOR_RED "RESULT: %d \n" COLOR_RESET, (stackElem_t)((double)(value) / 1000));

                break;
            }
            case MY_HLT: {

                printf("\nProgram ended\n");
                return ERR_OK;
            }
            case MY_JMP: {

                if (imm < 0 || (size_t)imm >= CPUData->stringsCount) return ERR_OUT_OF_BOUNDS;

                CPUData->IP = (size_t)imm;

                continue;
            }
            case MY_JA:
            case MY_JB:
            case MY_JAE:
            case MY_JBE:
            case MY_JE:
            case MY_JNE: {

                if (imm < 0 || (size_t)imm >= CPUData->stringsCount) {
                    return ERR_OUT_OF_BOUNDS;
                }

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    return ERR_INVALID_DATA;
                }

                stackElem_t el2 = 0;
                stackElem_t el1 = 0;

                stackPop(CPUData->stack, &el1);
                stackPop(CPUData->stack, &el2);

                if (jumpType(el1, el2, command)) {
                    CPU_DUMP(CPUData, CPUData->IP);
                    CPUData->IP = (size_t)imm;
                    continue;
                }

                break;
            }
            case MY_JT: {

                if (imm < 0 || (size_t)imm >= CPUData->stringsCount) {
                    return ERR_OUT_OF_BOUNDS;
                }
                bool jump = ifTuesday();

                if (jump) {
                    CPU_DUMP(CPUData, CPUData->IP);
                    CPUData->IP = (size_t)imm;
                    continue;
                }
                break;
            }
            case MY_CALL: {

                if (imm < 0 || (size_t)imm >= CPUData->stringsCount) return ERR_OUT_OF_BOUNDS;

                ErrorCode error = stackPush(&CPUData->returnAddresses, (stackElem_t)(CPUData->IP + 1));
                if (error != ERR_OK) return error;

                CPUData->IP = (size_t)imm;
                continue;
            }
            case MY_RET: {

                if (checkStackBeforeOperation(&CPUData->returnAddresses, 1) != ERR_OK) {
                    printf("ERROR: return stack is empty\n");
                    return ERR_INVALID_DATA;
                }

                stackElem_t returnAddress = 0;

                ErrorCode error = stackPop(&CPUData->returnAddresses, &returnAddress);
                if (error != ERR_OK) return error;

                CPUData->IP = (size_t)returnAddress;

                continue;
            }
            case MY_DRAW: {

                ErrorCode error = drawPixel(CPUData);
                if (error != ERR_OK) return error;

                break;
            }
            case MY_SHOWWINDOW: {

                createGraphicWindow();

                break;
            }
            default:{

                printf("Unknown command: %d\n", command);
                return ERR_INVALID_DATA;

            }
        }

        CPU_DUMP(CPUData, CPUData->IP);
        CPUData->IP++;

    }

    return ERR_INVALID_DATA;
}

void CPUDestroy(CPUInfo* CPUData)
{

    assert(CPUData);

    cleanRegister(CPUData);
    stackDestroy(CPUData->stack);
    stackDestroy(&CPUData->returnAddresses);

    free(CPUData->program);

    CPUData->program = NULL;
    CPUData->stack = NULL;
    CPUData->stringsCount = 0;
    CPUData->IP = 0;

    free(CPUData);

    printf(COLOR_RED "%s \n" COLOR_RESET, "CPU was destroyed");

    return;

};

ErrorCode cleanRegister(CPUInfo* CPUData){

    for(size_t i = 0; i < REGISTER_COUNT; i++){
        (CPUData->registers)[i] = -1;
    }

    return ERR_OK;

}

ErrorCode consoleInput(CPUInfo* CPUData){

    if(CPUData == NULL || CPUData->stack == NULL) return ERR_INVALID_ARGUMENT;

    int consoleArg = 0;
    printf("Enter the value: ");
    scanf("%d", &consoleArg);

    stackPush(CPUData->stack, consoleArg*1000);

    return ERR_OK;
}

bool jumpType(int el1, int el2, int command){

    bool jump = false;

    switch (command) {

        case MY_JA:  jump = (el1 >  el2); break;
        case MY_JB:  jump = (el1 <  el2); break;
        case MY_JAE: jump = (el1 >= el2); break;
        case MY_JBE: jump = (el1 <= el2); break;
        case MY_JE:  jump = (el1 == el2); break;
        case MY_JNE: jump = (el1 != el2); break;
        default: break;
    }

    return jump;
}

#ifdef CPU_DEBUG
ErrorCode CPUDump(const struct CPUInfo* CPUData, size_t currentLine){

    assert(CPUData);

    printf("\nCPUInfo '%s'[%p] created by %s() at %s:%u\n",
            CPUData->debugInfo.name,
            CPUData,
            CPUData->debugInfo.function,
            CPUData->debugInfo.file,
            CPUData->debugInfo.line);

    printf("{\n");

    printf("\tprogram[%p]\n", (const void*)CPUData->program);
    printf("\tcommandsCount = %u\n\n", CPUData->stringsCount);

    size_t start = CPUData->IP > 3 ? CPUData->IP - 3 : 0;
    size_t end = CPUData->IP + 4;

    for (size_t i = start; i < end; i++) {

        const binaryInstruction* instruction = &CPUData->program[i];

            if (i == currentLine) printf(BG_RED);

            printf("\t[%u] %d %d %d %d",
                   i,
                   instruction->opcode,
                   instruction->flags,
                   instruction->reg,
                   instruction->imm);

            if (i == currentLine)
                printf(COLOR_RESET);

            putchar('\n');
    }

    putchar('\n');

    printf("\tregisters[%p]{    \n", CPUData->registers);
    const char* registerNames[] = {"RAX", "RBX", "RCX", "RDX"};
    for (size_t i = 0; i < REGISTER_COUNT; i++) {
        printf("\t\t%s = %d\n", registerNames[i], CPUData->registers[i]);
    }

    printf("\t}\n\n");
    putchar('\n');

    printf("\n\tRAM[%p] {\n", (const void*)CPUData->RAM);

    for (size_t i = 0; i < RAM_CAPACITY; i++) {

        printf("\t\t[%u] = %d\n", i, CPUData->RAM[i]);
    }

    printf("\t}\n");


    printf("\tIP = %u \n", CPUData->IP);

    STACK_DUMP(CPUData->stack);

    printf("}\n");

    getchar();

    return ERR_OK;
}
#endif

// Drawing

void createGraphicWindow(){

    txCreateWindow(800, 600);

    txSetFillColor(TX_WHITE);
    txClear();

    txSetColor(TX_BLUE);
}

ErrorCode drawPixel(CPUInfo* CPUData) {

    assert(CPUData);

    const int WIDTH  = 800;
    const int HEIGHT = 600;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            int address = y * WIDTH + x;

            if (CPUData->RAM[address] != 0) {
                txSetPixel(x, y, TX_BLUE);
            }
        }
    }

    return ERR_OK;
}
