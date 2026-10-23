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

const int INVALID_REG   = 0xABADBABE;
const int INVALID_LABEL = 0xB16B00B5;

int main(int argc, char* argv[]){

    cancelBuffering();

    files usedFiles = {};

    ErrorCode error = setFileNames(&usedFiles, MAX_PATH_LENGTH, argc, argv);
    if(error) return error;

    // Assembler
    struct asmProgramInfo asmProgramData = {};

    error = asmLoad(&asmProgramData, usedFiles.asmProgramFile);
    if (error != ERR_OK) return error;

    #ifdef CPU_DEBUG
    consoleProgramOutput(&asmProgramData);
    #endif

    error = assembler(usedFiles.exeFile, usedFiles.binFile, &asmProgramData);
    if (error != ERR_OK) return error;

    printf("Asm completed successfully\n");

    return ERR_OK;
}

// Loads

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

ErrorCode readAsmProgram(const char* asmProgramFile, asmProgramInfo* asmProgramData){

    assert(asmProgramFile);
    assert(asmProgramData);

    ErrorCode error = readAsmTextIntoSingleBuffer(asmProgramFile, asmProgramData);
    if(error != ERR_OK) return error;

    asmProgramData->stringsCount = calculateStringsCount(asmProgramData->textOfAsmProgram);
    recordPtrStringsForAsm(asmProgramData);

    return ERR_OK;

}

ErrorCode readAsmTextIntoSingleBuffer(const char* asmProgramFile, struct asmProgramInfo* asmProgramData){

    assert(asmProgramFile);
    assert(asmProgramData);

    FILE* file = fopen(asmProgramFile, "r");

    if(file == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "ERR_FILE_NOT_FOUND", ERR_FILE_NOT_FOUND);
        printErrorIntoConsole(&debugLogInfo);
        printf("File %s not found\n", asmProgramFile);
        return ERR_FILE_NOT_FOUND;
    }

    struct stat fileInfo;
    stat(asmProgramFile, &fileInfo);
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

// Assembler-work

ErrorCode assembler(const char* exeFileName, const char* binFileName, struct asmProgramInfo* asmProgramData){

    assert(asmProgramData);
    assert(exeFileName);
    assert(binFileName);

    ErrorCode error = collectLabels(asmProgramData);
    if (error != ERR_OK) return error;

    FILE* exeFile = fopen(exeFileName, "w");

    if(exeFile == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Cannot open output file", ERR_FILE_NOT_FOUND);
        debugLogInfo.errCode = ERR_FILE_NOT_FOUND;
        printErrorIntoConsole(&debugLogInfo);

        return ERR_FILE_NOT_FOUND;
    };

    FILE* binFile = fopen(binFileName, "wb");

    if(binFile == NULL){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Cannot open output file", ERR_FILE_NOT_FOUND);
        debugLogInfo.errCode = ERR_FILE_NOT_FOUND;
        printErrorIntoConsole(&debugLogInfo);

        return ERR_FILE_NOT_FOUND;
    };

    hash_t hashesArr[COMMANDS_COUNT] = {};
    formCommandHashesMassive(hashesArr);
    for(size_t i = 0; i < asmProgramData->stringsCount; i++){

        char* line = asmProgramData->programLines[i];

        char commandName[MAX_COMMAND_LENGTH] = {};
        char argument[MAX_COMMAND_LENGTH] = {};

        int sscanfResult = sscanf((asmProgramData->programLines)[i], "%300s %300s", commandName, argument);

        if (sscanfResult <= 0) continue;
        if (commandName[0] == ':') continue;

        int commandIndex = findCommand(commandName, hashing, hashesArr);

        if (commandIndex == MY_PUSH) {

            int flags = 0;
            int reg = 0;
            int imm = 0;

            error = parsePushPopArguments(argument, &flags, &reg, &imm);
            if (error != ERR_OK) {
                FCLOSE(exeFile);
                FCLOSE(binFile);
                return error;
            }

            error = writeCommand(exeFile, binFile, MY_PUSH, flags, reg, imm, 4);
            if (error != ERR_OK) break;
        }

        else if (commandIndex == MY_POP) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 2, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            int flags = 0;
            int reg = 0;
            int imm = 0;

            error = parsePushPopArguments(argument, &flags, &reg, &imm);

            if (isPopAvailable(error, flags)) {

                error_debug debugLogInfo = {};
                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Incorrect POP command", ERR_INVALID_ARGUMENT);
                debugLogInfo.errCode = ERR_INVALID_ARGUMENT;
                printErrorIntoConsole(&debugLogInfo);
                printf("%s \n\n", line);

                FCLOSE(exeFile);
                FCLOSE(binFile);
                return ERR_INVALID_ARGUMENT;
            }

            error = writeCommand(exeFile, binFile, MY_POP, flags, reg, imm, 4);
            if (error != ERR_OK) break;
        }


        else if (commandIndex == MY_ADD) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_ADD, 0, 0, 0, 1);
            if (error != ERR_OK) break;
                    }
        else if (commandIndex == MY_SUB) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_SUB, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_MUL) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_MUL, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_DIV) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_DIV, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_SQUARE) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_SQUARE, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_SIN) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_SIN, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_COS){

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_COS, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_TG) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_TG, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_CTG) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_CTG, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_BREAK) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_BREAK, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_IN) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_IN, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_OUT) { // OUT

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_OUT, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_HLT) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_HLT, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_JMP) {

            error = checkAsmOperation(sscanfResult, exeFile, 2, __func__, __LINE__);
            if (error != ERR_OK) return error;

                        int address = findLabel(asmProgramData->labels, argument);

            if (address == INVALID_LABEL) {
                error_debug debugLogInfo = {};
                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Label-error", ERR_NOT_FOUND);
                printErrorIntoConsole(&debugLogInfo);
                printf("Unknown label: %s\n", argument);
                FCLOSE(exeFile);
                FCLOSE(binFile);
                return ERR_NOT_FOUND;
            }

            error = writeCommand(exeFile, binFile, MY_JMP, 0, 0, address, 2);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_CALL){

            error = checkAsmOperation(sscanfResult, exeFile, 2, __func__, __LINE__);
            if (error != ERR_OK) return error;

            int address = findLabel(asmProgramData->labels, argument);

            if (address == INVALID_LABEL) {
                error_debug debugLogInfo = {};
                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Label-error", ERR_NOT_FOUND);
                printErrorIntoConsole(&debugLogInfo);
                printf("Unknown label: %s\n", argument);
                FCLOSE(exeFile);
                FCLOSE(binFile);
                return ERR_NOT_FOUND;
            }

            error = writeCommand(exeFile, binFile, MY_CALL, 0, 0, address, 2);
            if (error != ERR_OK) break;
        }

        else if (commandIndex == MY_RET){

            ErrorCode checkOper = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            error = writeCommand(exeFile, binFile, MY_RET, 0, 0, 0, 1);
            if (error != ERR_OK) break;

        }
        else if (commandCondition (commandIndex)) {

            error = checkAsmOperation(sscanfResult, exeFile, 2, __func__, __LINE__);
            if (error != ERR_OK) return error;

            int address = findLabel(asmProgramData->labels, argument);

            if (address == INVALID_LABEL) {
                error_debug debugLogInfo = {};
                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Label-error", ERR_NOT_FOUND);
                printErrorIntoConsole(&debugLogInfo);
                printf("Unknown label: %s\n", argument);
                FCLOSE(exeFile);
                FCLOSE(binFile);
                return ERR_NOT_FOUND;
            }

            int command = 0;

            if      (commandIndex == MY_JA)  command = MY_JA;
            else if (commandIndex == MY_JB)  command = MY_JB;
            else if (commandIndex == MY_JAE)  command = MY_JAE;
            else if (commandIndex == MY_JBE)  command = MY_JBE;
            else if (commandIndex == MY_JE)  command = MY_JE;
            else if (commandIndex == MY_JNE)  command = MY_JNE;
            else if (commandIndex == MY_JT)  command = MY_JT;

            error = writeCommand(exeFile, binFile, command, 0, 0, address, 2);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_DRAW) {

            error = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);
            if (error != ERR_OK) break;

            error = writeCommand(exeFile, binFile, MY_DRAW, 0, 0, 0, 1);
            if (error != ERR_OK) break;
        }
        else if (commandIndex == MY_SHOWWINDOW) {

            error = checkAsmOperation(sscanfResult, exeFile, 1, __func__, __LINE__);

            if (error != ERR_OK) break;

            error = writeCommand(exeFile, binFile, MY_SHOWWINDOW, 0, 0, 0, 1);

            if (error != ERR_OK) break;
        }

        if (commandIndex == -1) {

            error_debug debugLogInfo = {};

            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__,
                                        "Unknown-command", ERR_INVALID_DATA);

            printErrorIntoConsole(&debugLogInfo);
            printf("Unknown command: %s\n", commandName);

            FCLOSE(exeFile);
            FCLOSE(binFile);

            return ERR_INVALID_DATA;
        }
    }

    FCLOSE(exeFile);
    FCLOSE(binFile);

    return ERR_OK;
}

ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line){

    assert(file);
    assert(function);
    assert(line > 0);

    if (sscanfResult != operationArgsCount) {
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo,
                            function,
                            line,
                            "The number of command arguments is incorrect",
                            ERR_INVALID_DATA);

        printErrorIntoConsole(&debugLogInfo);

        FCLOSE(file);
        return ERR_INVALID_DATA;
    }

    return ERR_OK;

}

bool commandCondition (int commandIndex){
    return (commandIndex == MY_JA)  ||
           (commandIndex == MY_JB)  ||
           (commandIndex == MY_JAE) ||
           (commandIndex == MY_JBE) ||
           (commandIndex == MY_JE)  ||
           (commandIndex == MY_JNE) ||
           (commandIndex == MY_JT);
}

void removeAsmComment(char* line) {

    assert(line);

    char* comment = strchr(line, ';');

    if (comment != NULL) {
        *comment = '\0';
    }

    return;
}

ErrorCode parsePushPopArguments(const char* arg, int* flags, int* reg, int* imm){

    assert(arg);
    assert(flags);
    assert(reg);
    assert(imm);

    char buffer[64] = {};

    if (strlen(arg) >= sizeof(buffer)){
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "PUSH-argument is too long", ERR_INVALID_ARGUMENT);
        printErrorIntoConsole(&debugLogInfo);

        return ERR_INVALID_ARGUMENT;
    }

    strcpy(buffer, arg);
    size_t bufLength = strlen(buffer);

    // PUSH [...] (RAM-PUSH)
    if(bufLength >= 2 && buffer[0] == '[' && buffer[bufLength-1] == ']'){

        *flags |= ARG_MEM;
        buffer[bufLength-1] ='\0';
        memmove(buffer, buffer + 1, bufLength - 1); // From buffer + 1 to buffer (bufLength - 1 bytes)
    }

    // PUSH AX+1
    char* plus = strchr(buffer, '+');
    if(plus != NULL){

        *plus = '\0';
        int registerIndex = getRegisterIndex(buffer);

        if(registerIndex == INVALID_REG) {
            error_debug debugLogInfo = {};
            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Invalid register", ERR_INVALID_ARGUMENT);
            printErrorIntoConsole(&debugLogInfo);

            return ERR_INVALID_ARGUMENT;
        }

        char rest ='\0';

        if (sscanf(plus + 1, "%d %c", imm, &rest) != 1) {
            error_debug debugLogInfo = {};
            ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Invalid register", ERR_INVALID_ARGUMENT);
            printErrorIntoConsole(&debugLogInfo);

            return ERR_INVALID_ARGUMENT;
        }

        *reg = registerIndex;
        *flags |= (ARG_REG | ARG_IMMED_VALUE);

        return ERR_OK;
    }

    // PUSH AX
    int registerIndex = getRegisterIndex(buffer);

    if (registerIndex != INVALID_REG) {

        *reg = registerIndex;
        *flags |= ARG_REG;

        return ERR_OK;
    }

    // PUSH 1
    char rest = '\0';

    if (sscanf(buffer, "%d %c", imm, &rest) != 1) return ERR_INVALID_ARGUMENT;

    *flags |= ARG_IMMED_VALUE;

    return ERR_OK;
}

ErrorCode writeCommand(FILE* exeFile, FILE* binFile,
                       int opcode, int flags, int reg, int imm,
                       int argsCount) {

    int32_t instruction[] = {opcode, flags, reg, imm};

    if (argsCount == 4) {

        fprintf(exeFile, "%d %d %d %d\n", opcode, flags, reg, imm);
    }
    else if (argsCount == 2) {

        fprintf(exeFile, "%d %d\n", opcode, imm);
    }
    else {

        fprintf(exeFile, "%d\n", opcode);
    }

    fwrite(instruction, sizeof(int32_t), 4, binFile);

    return ERR_OK;
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

            if (sscanf(line, "%300s", name) != 1) {
                error_debug debugLogInfo = {};
                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Incorrect label", ERR_INVALID_DATA);
                printErrorIntoConsole(&debugLogInfo);
                printf("Incorrect label in line: '%s'\n", line);

                return ERR_INVALID_DATA;
            }

            size_t nameLen = strlen(name);
            char* rest = line + nameLen;

            while (*rest == ' ' || *rest == '\t' || *rest == '\r') rest++;

            if (*rest != '\0'){
                error_debug debugLogInfo = {};
                ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Incorrect label ending", ERR_INVALID_DATA);
                printErrorIntoConsole(&debugLogInfo);
                printf("Incorrect label in line: '%s'\n", line);

                return ERR_INVALID_DATA;
            }

            ErrorCode error = addLabel(asmData->labels, line, commandAddress);
            if (error != ERR_OK) return error;

            line[nameLen] = '\0';

            continue;
        }

        commandAddress++;
    }

    return ERR_OK;
}

ErrorCode addLabel(label* labels, char* name, int address) {

    assert(labels);
    assert(name);
    assert(address >= 0);

    if (findLabel(labels, name) != INVALID_LABEL) {
        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "", ERR_ALREADY_EXISTS);
        printErrorIntoConsole(&debugLogInfo);
        printf("Error re-labeling: %s\n\n", name);

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

// RegisterIndex

int getRegisterIndex(const char* name) {

    if (strcmp(name, "RAX") == 0) return RAX;
    if (strcmp(name, "RBX") == 0) return RBX;
    if (strcmp(name, "RCX") == 0) return RCX;
    if (strcmp(name, "RDX") == 0) return RDX;

    return INVALID_REG;
}

// Hashing commands

hash_t hashing(const char* str) {

    assert(str);

    hash_t hash = 5381;

    while (*str != '\0') {
        hash = hash * 33 + (unsigned char)*str;
        str++;
    }

    return hash;
}

void formCommandHashesMassive(hash_t* hashesArr){

    assert(hashesArr);

    hash_t hashes[] = {hashing("PUSH"), hashing("ADD"), hashing("SUB"), hashing("MUL"), hashing("DIV"), hashing("SQUARE"),
                hashing("SIN"), hashing("COS"), hashing("TG"), hashing("CTG"), hashing("BREAK"), hashing("IN"),
                hashing("OUT"), hashing("HLT"), hashing("JMP"), hashing("JA"), hashing("JB"), hashing("JAE"),
                hashing("JBE"), hashing("JE"), hashing("JNE"), hashing("JT"), hashing("CALL"), hashing("RET"),
                hashing("POP"), hashing("DRAW"), hashing("SHOWWINDOW")};

    memcpy(hashesArr, hashes, sizeof(hashes));

    return;
}

int findCommand(const char* commandName, hashFunction_t hashFunction, hash_t* hashesArr) {

    assert(commandName);
    assert(hashFunction);
    assert(hashesArr);

    hash_t hash = hashFunction(commandName);

    if( hash == hashesArr[MY_PUSH - 1])     return MY_PUSH;
    if( hash == hashesArr[MY_POP - 1])        return MY_POP;
    if( hash == hashesArr[MY_ADD - 1])        return MY_ADD;
    if( hash == hashesArr[MY_SUB - 1])        return MY_SUB;
    if( hash == hashesArr[MY_MUL - 1])        return MY_MUL;
    if( hash == hashesArr[MY_DIV - 1])        return MY_DIV;
    if( hash == hashesArr[MY_SQUARE - 1])     return MY_SQUARE;
    if( hash == hashesArr[MY_SIN - 1])        return MY_SIN;
    if( hash == hashesArr[MY_COS - 1])        return MY_COS;
    if( hash == hashesArr[MY_TG - 1])         return MY_TG;
    if( hash == hashesArr[MY_CTG - 1])        return MY_CTG;
    if( hash == hashesArr[MY_BREAK - 1])      return MY_BREAK;
    if( hash == hashesArr[MY_IN - 1])         return MY_IN;
    if( hash == hashesArr[MY_OUT - 1])        return MY_OUT;
    if( hash == hashesArr[MY_HLT - 1])        return MY_HLT;
    if( hash == hashesArr[MY_JMP - 1])        return MY_JMP;
    if( hash == hashesArr[MY_CALL - 1])       return MY_CALL;
    if( hash == hashesArr[MY_RET - 1])        return MY_RET;
    if( hash == hashesArr[MY_JA - 1])         return MY_JA;
    if( hash == hashesArr[MY_JB - 1])         return MY_JB;
    if( hash == hashesArr[MY_JAE - 1])        return MY_JAE;
    if( hash == hashesArr[MY_JBE - 1])        return MY_JBE;
    if( hash == hashesArr[MY_JE - 1])         return MY_JE;
    if( hash == hashesArr[MY_JNE - 1])        return MY_JNE;
    if( hash == hashesArr[MY_JT - 1])         return MY_JT;
    if( hash == hashesArr[MY_DRAW - 1])       return MY_DRAW;
    if( hash == hashesArr[MY_SHOWWINDOW - 1]) return MY_SHOWWINDOW;

    return -1;
}

bool isPopAvailable(ErrorCode error, int32_t flags){
    return error != ERR_OK ||
                (flags !=  ARG_REG &&
                    flags != (ARG_MEM | ARG_REG) &&
                    flags != (ARG_MEM | ARG_IMMED_VALUE) &&
                    flags != (ARG_MEM | ARG_REG | ARG_IMMED_VALUE));
}
