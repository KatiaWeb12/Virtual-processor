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

const int INVALID_REG = 0xABADBABE;
const int INVALID_LABEL = 0xB16B00B5;

int main(int argc, char* argv[]){

    cancelBuffering();

    ErrorCode error = setFileNames(&usedFiles, MAX_PATH_LENGTH, argc, argv);
    if(error) return error;

    // Assembler
    struct asmProgramInfo asmProgramData = {};

    error = asmLoad(&asmProgramData, usedFiles.asmProgramFile);
    if (error != ERR_OK) return error;

    #ifdef CPU_DEBUG
    consoleProgramOutput(&asmProgramData);
    #endif

    error = assembler(usedFiles.exeFile, &asmProgramData);
    if (error != ERR_OK) return error;

    printf("Asm completed successfully\n");

    return ERR_OK;
}


ErrorCode asmLoad(asmProgramInfo* asmData, char* asmProgramFile){

    assert(asmData);
    assert(asmProgramFile);

    char** memory = (char**)calloc(MAX_COMMAND_LENGTH, sizeof(char*));
    if (memory == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "", ERR_OUT_OF_MEMORY);
        printErrorIntoConsole(&debugLogInfo);
        return ERR_OUT_OF_MEMORY;
    }

    asmData->programLines = memory;

    ErrorCode error = readAsmProgram(asmProgramFile, asmData);
    if (error != ERR_OK) return error;

    label* labelMemory = (label*)calloc(1, sizeof(label));
    if (labelMemory == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "", ERR_OUT_OF_MEMORY);
        printErrorIntoConsole(&debugLogInfo);
        return ERR_OUT_OF_MEMORY;
    }
    asmData->labels = labelMemory;

    error = loadLabels(asmData->labels);
    if (error != ERR_OK) return error;

    return ERR_OK;
}

ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData){

    assert(fileName);
    assert(asmProgramData);

    ErrorCode error = readAsmTextIntoSingleBuffer(fileName, asmProgramData);
    if(error != ERR_OK) return error;

    asmProgramData->stringsCount = calculateStringsCount(asmProgramData->textOfAsmProgram);
    recordPtrStringsForAsm(asmProgramData);

    return ERR_OK;

}

ErrorCode readAsmTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData){

    assert(fileName);
    assert(asmProgramData);

    FILE* file = fopen(fileName, "r");

    if(file == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "", ERR_FILE_NOT_FOUND);
        printErrorIntoConsole(&debugLogInfo);
        printf("File %s not found\n", fileName);
        return ERR_FILE_NOT_FOUND;
    }

    struct stat fileInfo;
    stat(fileName, &fileInfo);
    size_t fileSize = fileInfo.st_size;

    char* memory = (char*)calloc(fileSize + 1, sizeof(char));
    if(memory == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "", ERR_OUT_OF_MEMORY);
        printErrorIntoConsole(&debugLogInfo);
        return ERR_OUT_OF_MEMORY;
    };
    asmProgramData->textOfAsmProgram = memory;

    asmProgramData->textLength = fileSize;

    // Read file and write information to text
    fread(asmProgramData->textOfAsmProgram, fileSize, 1, file);
    (asmProgramData->textOfAsmProgram)[fileSize] = '\0';

    FCLOSE(file);

    return ERR_OK;
}

void recordPtrStringsForAsm(struct asmProgramInfo* asmProgramData){

    assert(asmProgramData);

    char* text = asmProgramData->textOfAsmProgram;
    char** programLines = asmProgramData->programLines;
    size_t stringsCount = asmProgramData->stringsCount;

    size_t stringIndex = 0;
    char* currentPtr = text;

    while(currentPtr != NULL && stringIndex < stringsCount){

        programLines[stringIndex++] = currentPtr;

        char* newString = strchr(currentPtr, '\n');
        if(newString == NULL) break;

        currentPtr = newString + 1;
        *newString = '\0';
    }

    return;
}

ErrorCode assembler(char* fileName, struct asmProgramInfo* asmProgramData){

    assert(asmProgramData);

    ErrorCode error = collectLabels(asmProgramData);
    if (error != ERR_OK) return error;

    FILE* file = fopen(fileName, "w");

    if(file == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Cannot open output file", ERR_FILE_NOT_FOUND);
        debugLogInfo.errCode = ERR_FILE_NOT_FOUND;
        printErrorIntoConsole(&debugLogInfo);
        return ERR_FILE_NOT_FOUND;
    };

    commandHash_t commandsArr[COMMANDS_COUNT];
    formCommandsArr(commandsArr);

    for(size_t i = 0; i < asmProgramData->stringsCount; i++){

        char* line = asmProgramData->programLines[i];
        removeAsmComment(line);

        char commandName[MAX_COMMAND_LENGTH] = {};
        char argument[MAX_COMMAND_LENGTH] = {};

        int sscanfResult = sscanf((asmProgramData->programLines)[i], "%300s %300s", commandName, argument);

        if (sscanfResult <= 0) continue;
        if (commandName[0] == ':') continue;

        hash_t commandHash = hashing(commandName);

        size_t commandIndex = 0;
        bool commandFound = false;

        while(commandIndex < COMMANDS_COUNT){
            if(commandsArr[commandIndex].hash == commandHash){
                if(strcmp(commandName, commandsArr[commandIndex].commandName) == 0){

                    commandFound = true;

                    if (commandIndex == 0) {
                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
                        if(checkOper != ERR_OK) {
                            continue;
                        };

                        int regIndex = getRegisterIndex(argument);

                        if (regIndex != INVALID_REG) {
                            fprintf(file, "%d %d\n", MY_PUSH_REG, regIndex);
                        }
                        else {
                            char* endPtr = NULL;
                            long value = strtol(argument, &endPtr, 10); //converts a string to an long integer

                            if (*endPtr != '\0') {
                                error_debug debugLogInfo = {};
                                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Incorrect register", ERR_INVALID_DATA);
                                printErrorIntoConsole(&debugLogInfo);
                                FCLOSE(file);
                                return ERR_INVALID_DATA;
                            }

                            fprintf(file, "%d %ld\n", MY_PUSH, value);
                        }
                    }

                    else if (commandIndex == 1) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        int regIndex = getRegisterIndex(argument);

                        if (regIndex == INVALID_REG) {
                            error_debug debugLogInfo = {};
                            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Non-register", ERR_INVALID_DATA);
                            printErrorIntoConsole(&debugLogInfo);
                            FCLOSE(file);
                            return ERR_INVALID_DATA;
                        }

                        fprintf(file, "%d %d\n", MY_POP_REG, regIndex);
                    }

                    else if (commandIndex == 2) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_ADD);
                    }
                    else if (commandIndex == 3) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_SUB);
                    }
                    else if (commandIndex == 4) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_MUL);
                    }
                    else if (commandIndex == 5) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_DIV);
                    }
                    else if (commandIndex == 6) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_SQUARE);
                    }
                    else if (commandIndex == 7) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_SIN);
                    }
                    else if (commandIndex == 8){

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_COS);
                    }
                    else if (commandIndex == 9) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_TG);
                    }
                    else if (commandIndex == 10) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_CTG);
                    }
                    else if (commandIndex == 11) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_BREAK);
                    }
                    else if (commandIndex == 12) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_IN);
                    }
                    else if (commandIndex == 13) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_OUT);
                    }
                    else if (commandIndex == 14) {

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;

                        fprintf(file, "%d\n", MY_HLT);
                    }
                    else if (commandIndex == 15) {

                        error = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
                        if (error != ERR_OK) return error;

                        int address = findLabel(asmProgramData->labels, argument);

                        if (address == INVALID_LABEL) {
                            error_debug debugLogInfo = {};
                            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Label-error", ERR_NOT_FOUND);
                            printErrorIntoConsole(&debugLogInfo);
                            printf("Unknown label: %s\n", argument);
                            FCLOSE(file);
                            return ERR_NOT_FOUND;
                        }

                        fprintf(file, "%d %d\n", MY_JMP, address);
                    }
                    else if (commandIndex == 16){

                        error = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
                        if (error != ERR_OK) return error;

                        int address = findLabel(asmProgramData->labels, argument);

                        if (address == INVALID_LABEL) {
                            error_debug debugLogInfo = {};
                            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Label-error", ERR_NOT_FOUND);
                            printErrorIntoConsole(&debugLogInfo);
                            printf("Unknown label: %s\n", argument);
                            FCLOSE(file);
                            return ERR_NOT_FOUND;
                        }

                        fprintf(file, "%d %d\n", MY_CALL, address);
                    }

                    else if (commandIndex == 17){

                        ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
                        if(checkOper != ERR_OK) return ERR_INVALID_DATA;
                        fprintf(file, "%d\n", MY_RET);

                    }
                    else if (commandCondition (commandIndex)) {

                        error = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
                        if (error != ERR_OK) return error;

                        int address = findLabel(asmProgramData->labels, argument);

                        if (address == INVALID_LABEL) {
                            error_debug debugLogInfo = {};
                            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Label-error", ERR_NOT_FOUND);
                            printErrorIntoConsole(&debugLogInfo);
                            printf("Unknown label: %s\n", argument);
                            FCLOSE(file);
                            return ERR_NOT_FOUND;
                        }

                        int command = 0;
                        if      (commandIndex == 18)  command = MY_JA;
                        else if (commandIndex == 19)  command = MY_JB;
                        else if (commandIndex == 20)  command = MY_JAE;
                        else if (commandIndex == 21)  command = MY_JBE;
                        else if (commandIndex == 22)  command = MY_JE;
                        else if (commandIndex == 23)  command = MY_JNE;
                        else if (commandIndex == 24)  command = MY_JT;

                        fprintf(file, "%d %d\n", command, address);
                    }
                    break;
                }
            }
            commandIndex++;
        }

        if(!commandFound) {

            error_debug debugLogInfo = {};
            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Unknown-command", ERR_INVALID_DATA);
            printErrorIntoConsole(&debugLogInfo);
            printf("Unknown command: %s\n", commandName);
            FCLOSE(file);

            return ERR_INVALID_DATA;
        }
    }

    FCLOSE(file);
    return ERR_OK;
}

ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line){

    assert(file);

    if (sscanfResult != operationArgsCount) {

        FCLOSE(file);

        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo,
                            function,
                            line,
                            "The number of command arguments is incorrect",
                            ERR_INVALID_DATA);

        printErrorIntoConsole(&debugLogInfo);

        return ERR_INVALID_DATA;
    }

    return ERR_OK;

}

bool commandCondition (int commandIndex){
    return  (commandIndex == 18)  ||
            (commandIndex == 19)  ||
            (commandIndex == 20)  ||
            (commandIndex == 21)  ||
            (commandIndex == 22)  ||
            (commandIndex == 23)  ||
            (commandIndex == 24);
}

void removeAsmComment(char* line) {

    assert(line);

    char* comment = strchr(line, ';');

    if (comment != NULL) {
        *comment = '\0';
    }

    return;
}

// Labels

ErrorCode loadLabels(label* labels){

    labels->labelCapacity = START_LABEL_COUNT;
    labels->labelCount = 0;

    labelItem* labelArrMemory = (labelItem*)calloc(START_LABEL_COUNT, sizeof(labelItem));
    if (labelArrMemory == NULL) return ERR_OUT_OF_MEMORY;

    labels->labelArr = labelArrMemory;

    for(size_t i = 0; i < START_LABEL_COUNT; i++){
        labels->labelArr[i].labelName = NULL;
        labels->labelArr[i].labelValue = -1;
    }

    return ERR_OK;
}

ErrorCode collectLabels(asmProgramInfo* asmData) {

    assert(asmData);
    assert(asmData->labels);

    int commandAddress = 0;

    // First pass to collect tags

    for (size_t i = 0; i < asmData->stringsCount; i++) {

        char* line = asmData->programLines[i]; // The line to go to

        removeAsmComment(line);

        while (*line == ' ' || *line == '\t' || *line == '\r') {
            line++;
        }

        if (*line == '\0') {
            continue;
        }

        if (*line == ':') {

            char name[MAX_COMMAND_LENGTH] = {};

            if (sscanf(line, "%299s", name) != 1) return ERR_INVALID_DATA;

            size_t nameLength = strlen(name);
            char* rest = line + nameLength;

            while (*rest == ' ' || *rest == '\t' || *rest == '\r') rest++;

            if (*rest != '\0') return ERR_INVALID_DATA;

            ErrorCode error = addLabel(asmData->labels, line, commandAddress);
            if (error != ERR_OK) return error;

            line[nameLength] = '\0';

            continue;
        }

        commandAddress++;
    }

    return ERR_OK;
}

ErrorCode addLabel(label* labels, char* name, int address) {

    if (labels == NULL || name == NULL || address < 0) return ERR_INVALID_ARGUMENT;

    if (findLabel(labels, name) != INVALID_LABEL) {
        printf("Error Re-labeling: %s\n", name);
        return ERR_ALREADY_EXISTS;
    }

    if (labels->labelCount == labels->labelCapacity) {
        ErrorCode error = resizeLabels(labels, 1);
        if (error != ERR_OK) return error;
    }

    size_t index = labels->labelCount;

    labels->labelArr[index].labelName = name;
    labels->labelArr[index].labelValue = address;

    labels->labelCount++;

    return ERR_OK;
}

int findLabel(const label* labels, const char* name) {

    assert(labels);
    assert(name);

    for (size_t i = 0; i < labels->labelCount; i++) {

        if (strcmp(labels->labelArr[i].labelName, name) == 0) {
            return labels->labelArr[i].labelValue;
        }
    }

    return INVALID_LABEL;
}

ErrorCode resizeLabels(label* labels, int direction){

    size_t oldCapacity = labels->labelCapacity;
    size_t newCapacity = 0;

    if (direction > 0){
        newCapacity = oldCapacity * 2;
    }
    else{
        if(oldCapacity <= 3){
            return ERR_OK;
        }
        newCapacity = oldCapacity / 2;
    }

    if(newCapacity == 0) return ERR_OUT_OF_BOUNDS;

    labelItem* newData = (labelItem*)realloc(labels->labelArr, newCapacity * sizeof(labelItem));

    if (newData == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    labels->labelArr = newData;

    labels->labelArr = (labelItem*)newData;
    labels->labelCapacity = newCapacity;

    if (newCapacity > oldCapacity) {

        for (size_t i = oldCapacity; i < newCapacity; i++) {
            labels->labelArr[i].labelName = NULL;
            labels->labelArr[i].labelValue = -1;
        }

    }

    #ifdef CPU_DEBUG
    printf(COLOR_RED "%s" COLOR_RESET, "\nLabel array resize\n");
    #endif

    return ERR_OK;
}

int getRegisterIndex(const char* name) {

    if (strcmp(name, "RAX") == 0) return RAX;
    if (strcmp(name, "RBX") == 0) return RBX;
    if (strcmp(name, "RCX") == 0) return RCX;
    if (strcmp(name, "RDX") == 0) return RDX;

    return INVALID_REG;
}

hash_t hashing(const char* str) {

    assert(str);

    hash_t hash = 5381;

    while (*str != '\0') {
        hash = hash * 33 + (unsigned char)*str;
        str++;
    }

    return hash;
}

void formCommandsArr(commandHash_t* commandsArr){

    const char* commandsNames[] = {"PUSH", "POP", "ADD", "SUB", "MUL", "DIV", "SQUARE", "SIN", "COS",
                                   "TG", "CTG", "BREAK", "IN", "OUT", "HLT", "JMP", "CALL", "RET",
                                   "JA", "JB", "JAE", "JBE", "JE", "JNE", "JT"};

    for(size_t i = 0; i < sizeof(commandsNames) / sizeof(commandsNames[0]); i++){
        createCommandHashEl(commandsArr, commandsNames[i], i);
    }

}

void createCommandHashEl(commandHash_t* commandsArr, const char* command, size_t commandIndex){

    assert(command);

    commandsArr[commandIndex] = {};
    commandsArr[commandIndex].commandName = command;
    commandsArr[commandIndex].hash = hashing(command);

    return;

}

