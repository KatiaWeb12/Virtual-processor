#include <TXLib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

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
    ErrorCode error = setExeFilename(exeFile, MAX_PATH_LENGTH, argc, argv);
    if(error) return error;

    struct CPUInfo CPUData = {};
    struct stack_t stk = {};

    error = CPULoad(&CPUData, &stk, exeFile);
    if (error != ERR_OK) return error;

    #ifdef CPU_DEBUG
        CPUData.debugInfo = {
            "CPUData",
            __FILE__,
            __func__,
            __LINE__
        };
    #endif

    error = CPUexecute(&CPUData);
    if(error != ERR_OK) return error;

    CPUDestroy(&CPUData);

    return ERR_OK;
}

ErrorCode CPULoad(CPUInfo* CPUData, stack_t* stk, char* exeProgramFile){

    char** memory = (char**)calloc(MAX_COMMAND_LENGTH, sizeof(char*));
    if (memory == NULL) return ERR_OUT_OF_MEMORY;

    CPUData->programLines = memory;

    readExeProgram(exeProgramFile, CPUData);

    ErrorCode error = STACK_INIT(stk, 3);
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

ErrorCode readExeProgram(const char* fileName, CPUInfo* CPUData){

    readExeTextIntoSingleBuffer(fileName, CPUData);
    CPUData->stringsCount = calculateStringsCount(CPUData->textOfExeProgram);
    recordPtrStringsForExe(CPUData); //TODO: ссылки

    return ERR_OK;

}

ErrorCode readExeTextIntoSingleBuffer(const char* fileName, CPUInfo* CPUData){

    FILE* file = fopen(fileName, "r");
    if(file == NULL) return ERR_UNKNOWN;

    struct stat fileInfo;
    stat(fileName, &fileInfo);
    size_t fileSize = fileInfo.st_size;

    char* memory = (char*)calloc(fileSize + 1, sizeof(char));
    if(memory == NULL) return ERR_OUT_OF_MEMORY;
    CPUData->textOfExeProgram = memory;

    CPUData->textLength = fileSize;

    fread(CPUData->textOfExeProgram, fileSize, 1, file);
    (CPUData->textOfExeProgram)[fileSize] = '\0';

    FCLOSE(file);

    return ERR_OK;
}

ErrorCode recordPtrStringsForExe(CPUInfo* CPUData){

    if(CPUData == NULL) return ERR_INVALID_ARGUMENT;

    char* text = CPUData->textOfExeProgram;
    char** programLines = CPUData->programLines;
    size_t stringsCount = CPUData->stringsCount;

    size_t stringIndex = 0;
    char* currentPtr = text;

    while(currentPtr != NULL && stringIndex < stringsCount){

        programLines[stringIndex++] = currentPtr;

        char* newString = strchr(currentPtr, '\n');
        if(newString == NULL) break;

        currentPtr = newString + 1;
        *newString = '\0';
    }

    return ERR_OK;
}

ErrorCode CPUexecute(CPUInfo* CPUData){

    int command = 0;
    int arg = 0;

    while (CPUData->IP < CPUData->stringsCount) {

        int sscanfResult = sscanf(CPUData->programLines[CPUData->IP], "%d %d", &command, &arg);
        if (sscanfResult < 1) return ERR_INVALID_DATA;

        switch(command){

            case MY_PUSH:{

                stackPush(CPUData->stack, arg * 1000);
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

                if (sscanfResult != 2) {
                    return ERR_INVALID_DATA;
                }

                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) {
                    return ERR_OUT_OF_BOUNDS;
                }
                #ifdef CPU_DEBUG
                printf("A new iteration of the cycle\n");
                #endif

                CPUData->IP = (size_t)arg;
                continue;
            }
            case MY_PUSH_REG: {

                if (arg < 0 || arg >= REGISTER_COUNT) {
                    return ERR_INVALID_ARGUMENT;
                }

                ErrorCode error = stackPush(CPUData->stack, CPUData->registers[arg]);
                if (error != ERR_OK) {
                    return error;
                }

                break;
            }
            case MY_POP_REG: {

                if (arg < 0 || arg >= REGISTER_COUNT) {
                    return ERR_INVALID_ARGUMENT;
                }

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    return ERR_INVALID_DATA;
                }

                ErrorCode error = stackPop(CPUData->stack, &(CPUData->registers[arg]));

                if (error != ERR_OK) {
                    return error;
                }

                break;
            }
            case MY_JA:
            case MY_JB:
            case MY_JAE:
            case MY_JBE:
            case MY_JE:
            case MY_JNE: {

                if (sscanfResult != 2) {
                    return ERR_INVALID_DATA;
                }

                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) {
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
                    CPUData->IP = (size_t)arg;
                    continue;
                }

                break;
            }
            case MY_JT: {

                if (sscanfResult != 2) {
                    return ERR_INVALID_DATA;
                }
                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) {
                    return ERR_OUT_OF_BOUNDS;
                }
                bool jump = ifTuesday();

                if (jump) {
                    CPU_DUMP(CPUData, CPUData->IP);
                    CPUData->IP = (size_t)arg;
                    continue;
                }
                break;
            }
            case MY_CALL: {

                if (sscanfResult != 2) return ERR_INVALID_DATA;
                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) return ERR_OUT_OF_BOUNDS;

                ErrorCode error = stackPush(&CPUData->returnAddresses, (stackElem_t)(CPUData->IP + 1));
                if (error != ERR_OK) return error;

                CPUData->IP = (size_t)arg;
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

    CPU_DUMP(CPUData, CPUData->stringsCount-1);

    cleanRegister(CPUData);
    stackDestroy(CPUData->stack);
    stackDestroy(&CPUData->returnAddresses);

    char* dataMemory = (char*)CPUData->programLines; //TODO: в free
    free(dataMemory);

    CPUData->textLength = 0;
    CPUData->stringsCount = 0;
    CPUData->IP = 0;

    free(CPUData->textOfExeProgram);
    CPUData->programLines = NULL;

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

    if (CPUData == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    printf("\nCPUInfo '%s'[%p] created by %s() at %s:%u\n",
            CPUData->debugInfo.name,
            CPUData,
            CPUData->debugInfo.function,
            CPUData->debugInfo.file,
            CPUData->debugInfo.line);

    printf("{\n");

    printf("\ttextLength = %u \n", CPUData->textLength);
    printf("\tstringsCount = %u \n", CPUData->stringsCount);

    printf("\ttextOfExeProgram[%p]: ", CPUData->textOfExeProgram);

    for (size_t i = 0; i < CPUData->stringsCount; i++) {

        if (i == currentLine) {
            printf(BG_RED " %s " COLOR_RESET "|", CPUData->programLines[i]);
        }
        else {
            printf(" %s | ", CPUData->programLines[i]);
        }
    }

    putchar('\n');

    printf("\tregisters[%p]{    \n", CPUData->registers);

    for(size_t i = 0; i < REGISTER_COUNT; i++){
        printStackElem(i, (CPUData->registers)[i]);
    }

    printf("\t}\n\n");
    putchar('\n');

    printf("\tIP = %u \n", CPUData->IP);

    STACK_DUMP(CPUData->stack);

    printf("}\n");

    getchar();

    return ERR_OK;
}
#endif
