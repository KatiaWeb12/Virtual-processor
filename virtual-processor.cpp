#include <TXLib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../universal-features/error.h"
#include "../universal-features/colors.h"
#include "../Task5 stack/stack.cpp"
#include "virtual-processor.h"
#include "math-functions.cpp"
#include "extra-functions.cpp"


int main(int argc, char* argv[]){

    cancelBuffering();

    struct files usedFiles = {};
    setFileNames(&usedFiles, MAX_PATH_LENGTH, argc, argv);

    // Assembler
    struct asmProgramInfo asmProgramData = {};

    char** memory = (char**)calloc(MAX_COMMAND_LENGTH, sizeof(char*));
    if (memory == NULL) return ERR_OUT_OF_MEMORY;

    asmProgramData.programLines = memory;

    readAsmProgram(usedFiles.asmProgramFile, &asmProgramData);
    consoleProgramOutput(&asmProgramData);
    assembler(usedFiles.exeFile, &asmProgramData);

    //Processor
    struct CPUInfo CPUData = {};
    struct stack_t stk = {};

    CPULoad(&CPUData, &stk, usedFiles.exeFile);

    #ifdef CPU_DEBUG
        CPUData.debugInfo = {
            "CPUData",
            __FILE__,
            __func__,
            __LINE__
        };
    #endif

    CPUexecute(usedFiles.exeFile, &CPUData);

    CPUDestroy(&CPUData);

    return 0;
}

size_t calculateStringsCount(const char* text){

    size_t stringCount = 0;
    const char* symbolPtr = text;

    while(*symbolPtr){
        if(*symbolPtr == '\n'){
            stringCount++;
        }
        symbolPtr++;
    }

    return stringCount;
}


// Assembler

ErrorCode readAsmTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData){

    FILE* file = fopen(fileName, "r");
    if(file == NULL) return ERR_UNKNOWN;

    struct stat fileInfo;
    stat(fileName, &fileInfo);
    size_t fileSize = fileInfo.st_size;

    char* memory = (char*)calloc(fileSize + 1, sizeof(char));
    if(memory == NULL) return ERR_OUT_OF_MEMORY;
    asmProgramData->textOfAsmProgram = memory;

    asmProgramData->textLength = fileSize;

    // Read file and write information to text
    fread(asmProgramData->textOfAsmProgram, fileSize, 1, file);
    (asmProgramData->textOfAsmProgram)[fileSize] = '\0';

    if (fclose(file) != 0) {
        printf("Warning: the file wasn't closed");
    }

    return ERR_OK;
}

ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData){

    readAsmTextIntoSingleBuffer(fileName, asmProgramData);
    asmProgramData->stringsCount = calculateStringsCount(asmProgramData->textOfAsmProgram);
    recordPtrStringsForAsm(asmProgramData);

    return ERR_OK;

}

ErrorCode recordPtrStringsForAsm(struct asmProgramInfo* asmProgramData){

    if(asmProgramData == NULL) return ERR_INVALID_ARGUMENT;

    char* text = asmProgramData->textOfAsmProgram;
    char** programLines = asmProgramData->programLines;
    size_t stringsCount = asmProgramData->stringsCount;

    size_t stringIndex = 0;
    char* currentPtr = text;

    while(currentPtr != NULL && stringIndex < stringsCount){

        programLines[stringIndex] = currentPtr;

        char* newString = strchr(currentPtr, '\n');
        if(newString == NULL) break;

        currentPtr = newString + 1;
        *newString = '\0';
        stringIndex++;
    }

    return ERR_OK;
}

ErrorCode assembler(char* fileName, struct asmProgramInfo* asmProgramData){

    if(asmProgramData == NULL) return ERR_INVALID_ARGUMENT;

    FILE* file = fopen(fileName, "w");

    if(file == NULL) return ERR_UNKNOWN;

    for(size_t i = 0; i < asmProgramData->stringsCount; i++){

        char commandName[MAX_COMMAND_LENGTH];
        char argument[MAX_COMMAND_LENGTH];

        int sscanfResult = sscanf((asmProgramData->programLines)[i], "%299s %299s", commandName, argument);

        if(sscanfResult == 0) continue;

        if (strcmp(commandName, "PUSH") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            int regIndex = getRegisterIndex(argument);

            if (regIndex != -1) {
                fprintf(file, "%d %d\n", MY_PUSH_REG, regIndex);
            }
            else {
                char* endPtr = NULL;
                long value = strtol(argument, &endPtr, 10);

                if (*endPtr != '\0') {
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                fprintf(file, "%d %ld\n", MY_PUSH, value);
            }
        }

        else if (strcmp(commandName, "POP") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            int regIndex = getRegisterIndex(argument);

            if (regIndex == -1) {
                if (fclose(file) != 0) {
                    printf("Warning: the file wasn't closed");
                }
                return ERR_INVALID_ARGUMENT;
            }

            fprintf(file, "%d %d\n", MY_POP_REG, regIndex);
        }

        else if (strcmp(commandName, "ADD") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_ADD);
        }
        else if (strcmp(commandName, "SUB") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_SUB);
        }
        else if (strcmp(commandName, "MUL") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_MUL);
        }
        else if (strcmp(commandName, "DIV") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_DIV);
        }
        else if (strcmp(commandName, "SQUARE") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_SQUARE);
        }
        else if (strcmp(commandName, "SIN") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_SIN);
        }
        else if (strcmp(commandName, "COS") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_COS);
        }
        else if (strcmp(commandName, "TG") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_TG);
        }
        else if (strcmp(commandName, "CTG") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_CTG);
        }
        else if (strcmp(commandName, "BREAK") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_BREAK);
        }
        else if (strcmp(commandName, "OUT") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_OUT);
        }
        else if (strcmp(commandName, "HLT") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_HLT);
        }
        else {
            printf("Unknown command: %s\n", commandName);

            if (fclose(file) != 0) {
                printf("Warning: the file wasn't closed");
            }
            return ERR_INVALID_DATA;
        }
    }

    if (fclose(file) != 0) {
        printf("Warning: the file wasn't closed");
    }
    return ERR_OK;
}

// Processor

ErrorCode readExeProgram(const char* fileName, CPUInfo* CPUData){

    readExeTextIntoSingleBuffer(fileName, CPUData);
    CPUData->stringsCount = calculateStringsCount(CPUData->textOfExeProgram);
    recordPtrStringsForExe(CPUData);

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

    if (fclose(file) != 0) {
        printf("Warning: the file wasn't closed");
    }

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

        programLines[stringIndex] = currentPtr;

        char* newString = strchr(currentPtr, '\n');
        if(newString == NULL) break;

        currentPtr = newString + 1;
        stringIndex++;
        *newString = '\0';
    }

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
    CPUData->IP = 0;

    return ERR_OK;
}

ErrorCode CPUDestroy(CPUInfo* CPUData)
{

    CPU_DUMP(CPUData, CPUData->stringsCount-1);

    cleanRegister(CPUData);
    stackDestroy(CPUData->stack);

    char* dataMemory = (char*)CPUData->programLines;
    free(dataMemory);

    CPUData->textLength = 0;
    CPUData->stringsCount = 0;
    CPUData->IP = 0;
    CPUData->textOfExeProgram = NULL;
    CPUData->programLines = NULL;

    printf(COLOR_RED "%s \n" COLOR_RESET, "CPU was destroyed");

    return ERR_OK;

};

ErrorCode cleanRegister(CPUInfo* CPUData){

    for(size_t i = 0; i < REGISTER_COUNT; i++){
        (CPUData->registers)[i] = -1;
    }

    return ERR_OK;

}

ErrorCode CPUexecute(char* fileName, CPUInfo* CPUData){

    FILE* file = fopen(fileName, "r");
    if(file == NULL) return ERR_UNKNOWN;

    char line[MAX_COMMAND_LENGTH] = {};

    int command = 0;
    int arg = 0;

    size_t currentLine = 0; // for DUMP

    while (command != MY_HLT) {

        if (fgets(line, MAX_COMMAND_LENGTH, file) == NULL) break;
        sscanf(line, "%d %d", &command, &arg);

        if (command == MY_HLT) break;

        switch(command){

            case MY_PUSH:{

                stackPush(CPUData->stack, arg * 1000);
                break;
            }

            case MY_ADD:{

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                addCommand(CPUData->stack);

                break;
            }

            case MY_SUB: {

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                subCommand(CPUData->stack);

                break;
            }

            case MY_MUL:{

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                mulCommand(CPUData->stack);

                break;
            }

            case MY_DIV: {

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                divCommand(CPUData->stack);

                break;
            }

            case MY_SQUARE: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                squareCommand(CPUData->stack);

                break;
            }

            case MY_SIN:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                sinCommand(CPUData->stack);

                break;
            }

            case MY_COS:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                cosCommand(CPUData->stack);

                break;
            }

            case MY_TG: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                tgCommand(CPUData->stack);

                break;
            }

            case MY_CTG: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                ctgCommand(CPUData->stack);

                break;
            }

            case MY_BREAK:{
                *getRightDataCanary(CPUData->stack) = 8;
            }

            case MY_OUT:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                stackElem_t value = 0;
                stackPop(CPUData->stack, &value);
                printf("\nRESULT: %d \n", (stackElem_t)((double)(value) / 1000));

                break;
            }
            case MY_HLT: {

                printf("Program ended");
                if (fclose(file) != 0) {
                    printf("Warning: the file wasn't closed");
                }
                return ERR_OK;
            }
            case MY_PUSH_REG: {

                if (arg < 0 || arg >= REGISTER_COUNT) {
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_ARGUMENT;
                }

                ErrorCode error = stackPush(CPUData->stack, CPUData->registers[arg]);

                if (error != ERR_OK) {
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return error;
                }

                break;
            }
            case MY_POP_REG: {

                if (arg < 0 || arg >= REGISTER_COUNT) {
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_ARGUMENT;
                }

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                ErrorCode error = stackPop(CPUData->stack, &(CPUData->registers[arg]));

                if (error != ERR_OK) {
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return error;
                }

                break;
            }
            default:{

                printf("Unknown command: %d\n", command);
                if (fclose(file) != 0) {
                    printf("Warning: the file wasn't closed");
                }
                return ERR_INVALID_DATA;

            }
        }

        CPU_DUMP(CPUData, currentLine);
        currentLine++;

    }

    return ERR_INVALID_DATA;
}

int getRegisterIndex(const char* name) {

    if (strcmp(name, "RAX") == 0) return RAX;
    if (strcmp(name, "RBX") == 0) return RBX;
    if (strcmp(name, "RCX") == 0) return RCX;
    if (strcmp(name, "RDX") == 0) return RDX;

    return -1;
}

#ifdef CPU_DEBUG
ErrorCode CPUDump(const struct CPUInfo* CPUData, size_t currentLine){

    if (CPUData == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    printf("\nstack_t '%s'[%p] created by %s() at %s:%u\n",
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
            printf(COLOR_RED "%s" COLOR_RESET " | ", CPUData->programLines[i]);
        }
        else {
            printf(" %s | ", CPUData->programLines[i]);
        }
    }

    putchar('\n');

    printf("\tregisters[%p]{    \n", CPUData->registers);

    for(size_t i = 0; i < REGISTER_COUNT; i++){
        printf("\t\t%d\n", CPUData->registers[i]);
    }

    printf("\t}\n\n");
    putchar('\n');

    STACK_DUMP(CPUData->stack);

    printf("}\n");

    getchar();

    return ERR_OK;
}
#endif
