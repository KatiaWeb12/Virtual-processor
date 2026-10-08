#include <TXLib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>

#include "../universal-features/error.h"
#include "../Task5 stack/stack.cpp"
#include "virtual-processor.h"
#include "math-functions.cpp"

int main(int argc, char* argv[]){

    cancelBuffering();

    struct files usedFiles = {};
    setFileNames(&usedFiles, MAX_PATH_LENGTH, argc, argv);

    struct asmProgramInfo asmProgramData = {};

    char** memory = (char**)calloc(MAX_COMMAND_LENGTH, sizeof(char*));
    if (memory == NULL) return ERR_OUT_OF_MEMORY;

    asmProgramData.programLines = memory;

    readAsmProgram(usedFiles.asmProgramFile, &asmProgramData);

    consoleProgramOutput(&asmProgramData);

    assembler(usedFiles.exeFile, &asmProgramData);

    struct stack_t stk = {};
    ErrorCode error = STACK_INIT(&stk, 3);
    if(error){
        printf("The function stackInit was completed with an error code: %d\n", error);
        return error;
    }

    CPUexecute(usedFiles.exeFile, &stk);

    return 0;
}

ErrorCode readTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData){

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

    readTextIntoSingleBuffer(fileName, asmProgramData);
    asmProgramData->stringsCount = calculateStringsCount(asmProgramData->textOfAsmProgram);
    recordPtrStrings(asmProgramData);

    return ERR_OK;

}

ErrorCode recordPtrStrings(struct asmProgramInfo* asmProgramData){

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

ErrorCode assembler(char* fileName, struct asmProgramInfo* asmProgramData){

    if(asmProgramData == NULL) return ERR_INVALID_ARGUMENT;

    FILE* file = fopen(fileName, "w");

    if(file == NULL) return ERR_UNKNOWN;

    for(size_t i = 0; i < asmProgramData->stringsCount; i++){

        char command[MAX_COMMAND_LENGTH];
        size_t argument = 0;
        int sscanfResult = sscanf((asmProgramData->programLines)[i], "%s %d", command, &argument);

        if(sscanfResult == 0) continue;

        if (strcmp(command, "PUSH") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d %d\n", MY_PUSH, argument);
        }
        else if (strcmp(command, "ADD") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_ADD);
        }
        else if (strcmp(command, "SUB") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_SUB);
        }
        else if (strcmp(command, "MUL") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_MUL);
        }
        else if (strcmp(command, "DIV") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_DIV);
        }
        else if (strcmp(command, "SQUARE") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_SQUARE);
        }
        else if (strcmp(command, "SIN") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_SIN);
        }
        else if (strcmp(command, "COS") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_COS);
        }
        else if (strcmp(command, "TG") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_TG);
        }
        else if (strcmp(command, "CTG") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_CTG);
        }
        else if (strcmp(command, "BREAK") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_BREAK);
        }
        else if (strcmp(command, "OUT") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_OUT);
        }
        else if (strcmp(command, "HLT") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_HLT);
        }
        else {
            printf("Unknown command: %s\n", command);

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

ErrorCode CPUexecute(char* fileName, stack_t* stack){

    FILE* file = fopen(fileName, "r");
    if(file == NULL) return ERR_UNKNOWN;

    char line[MAX_COMMAND_LENGTH] = {};

    int command = 0;
    int arg = 0;

    while (command != MY_HLT) {

        if (fgets(line, MAX_COMMAND_LENGTH, file) == NULL) break;
        sscanf(line, "%d %d", &command, &arg);

        if (command == MY_HLT) break;

        switch(command){

            case MY_PUSH:{

                stackPush(stack, arg * 1000);
                break;
            }

            case MY_ADD:{

                if (checkStackBeforeOperation(stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                addCommand(stack);

                break;
            }

            case MY_SUB: {

                if (checkStackBeforeOperation(stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                subCommand(stack);

                break;
            }

            case MY_MUL:{

                if (checkStackBeforeOperation(stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                mulCommand(stack);

                break;
            }

            case MY_DIV: {

                if (checkStackBeforeOperation(stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                divCommand(stack);

                break;
            }

            case MY_SQUARE: {

                if (checkStackBeforeOperation(stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                squareCommand(stack);

                break;
            }

            case MY_SIN:{

                if (checkStackBeforeOperation(stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                sinCommand(stack);

                break;
            }

            case MY_COS:{

                if (checkStackBeforeOperation(stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                cosCommand(stack);

                break;
            }

            case MY_TG: {

                if (checkStackBeforeOperation(stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                tgCommand(stack);

                break;
            }

            case MY_CTG: {

                if (checkStackBeforeOperation(stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                ctgCommand(stack);

                break;
            }

            case MY_BREAK:{
                *getRightDataCanary(stack) = 8;
            }

            case MY_OUT:{

                if (checkStackBeforeOperation(stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    if (fclose(file) != 0) {
                        printf("Warning: the file wasn't closed");
                    }
                    return ERR_INVALID_DATA;
                }

                stackElem_t value = 0;
                stackPop(stack, &value);
                stackPush(stack, value);
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
            default:{

                printf("Unknown command: %d\n", command);
                if (fclose(file) != 0) {
                    printf("Warning: the file wasn't closed");
                }
                return ERR_INVALID_DATA;

            }
        }

    }

    return ERR_INVALID_DATA;
}

ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount){

    if(stack == NULL) return ERR_INVALID_ARGUMENT;

    if(stack->size < argCount) return ERR_INVALID_DATA;

    return ERR_OK;
}

ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line){

    if (sscanfResult != operationArgsCount) {
        if (fclose(file) != 0) {
            printf("Warning: the file wasn't closed");
        }

        debugLog_t debugLogInfo = {};
        LOG_STRUCT_FORMAT((&debugLogInfo), stk, function, line);
        debugLogInfo.error = ERR_INVALID_DATA;
        printErrorIntoConsole(&debugLogInfo);

        return ERR_INVALID_DATA;
    }

    return ERR_OK;

}

ErrorCode printErrorIntoConsole(struct debugLog_t* debugLogInfo) {

    if(debugLogInfo == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    printf("[%s] Function '%s' was completed with Error %d in file '%s' in line %d \n",
            debugLogInfo->time,
            debugLogInfo->function,
            debugLogInfo->error,
            debugLogInfo->file,
            debugLogInfo->line);

    return ERR_OK;
}

void setFileNames(struct files* usedFiles, const size_t maxPathLength, int argc, char* argv[]){

    if(argc != 3){
        printf("Input-Error\n");
        printf("Type: path, input file, output file.\nPaths must be no longer than %d characters.\n", maxPathLength);
        exit(1);
    }

    const char* PATH = "./used-files/";

    sprintf(usedFiles->path, "%s", PATH);

    sprintf(usedFiles->asmProgramFile, "%s%s",
            PATH, argv[1]);

    sprintf(usedFiles->exeFile, "%s%s",
            PATH, argv[2]);

    printf("Path:        [%s]\n", usedFiles->path);
    printf("Input file:  [%s]\n", usedFiles->asmProgramFile);
    printf("Result file: [%s]\n", usedFiles->exeFile);

    return;
}

void cancelBuffering(){

    // Abandoning standard buffering since a custom buffer already exists
    setvbuf(stdout, NULL, _IONBF, 0);

    return;
}

ErrorCode consoleProgramOutput(struct asmProgramInfo* asmProgramData){

    if(asmProgramData == NULL) return ERR_INVALID_ARGUMENT;

    printf("\nConsole program commands printing:\n");
    for(size_t i = 0; i < asmProgramData->stringsCount; i++){
        printf("%s \n", asmProgramData->programLines[i]);
    }
    putchar('\n');
    return ERR_OK;
}

