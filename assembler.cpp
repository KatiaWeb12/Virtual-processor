#include "../universal-features/error.h"

ErrorCode asmLoad(asmProgramInfo* asmData, char* asmProgramFile){

    if (asmData == NULL || asmProgramFile == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    char** memory = (char**)calloc(MAX_COMMAND_LENGTH, sizeof(char*));
    if (memory == NULL) return ERR_OUT_OF_MEMORY;

    asmData->programLines = memory;

    ErrorCode error = readAsmProgram(asmProgramFile, asmData);

    if (error != ERR_OK) return error;

    label* labelMemory = (label*)calloc(1, sizeof(label));
    if (labelMemory == NULL) return ERR_OUT_OF_MEMORY;
    asmData->labels = labelMemory;

    error = loadLabels(asmData->labels);
    if (error != ERR_OK) return error;

    return ERR_OK;
}

ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData){

    readAsmTextIntoSingleBuffer(fileName, asmProgramData);
    asmProgramData->stringsCount = calculateStringsCount(asmProgramData->textOfAsmProgram);
    recordPtrStringsForAsm(asmProgramData);

    return ERR_OK;

}

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

    FCLOSE(file);

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

        programLines[stringIndex++] = currentPtr;

        char* newString = strchr(currentPtr, '\n');
        if(newString == NULL) break;

        currentPtr = newString + 1;
        *newString = '\0';
    }

    return ERR_OK;
}

ErrorCode assembler(char* fileName, struct asmProgramInfo* asmProgramData){

    if(asmProgramData == NULL) return ERR_INVALID_ARGUMENT;

    ErrorCode error = collectLabels(asmProgramData);
    if (error != ERR_OK) return error;

    FILE* file = fopen(fileName, "w");

    if(file == NULL) return ERR_UNKNOWN;

    for(size_t i = 0; i < asmProgramData->stringsCount; i++){

        char commandName[MAX_COMMAND_LENGTH] = {};
        char argument[MAX_COMMAND_LENGTH] = {};

        int sscanfResult = sscanf((asmProgramData->programLines)[i], "%299s %299s", commandName, argument);

        if (sscanfResult <= 0) continue;
        if (commandName[0] == ':') continue;

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
                    FCLOSE(file);
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
                FCLOSE(file);
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
        else if (strcmp(commandName, "IN") == 0) {

            ErrorCode checkOper = checkAsmOperation(sscanfResult, file, 1, __func__, __LINE__);
            if(checkOper != ERR_OK) return ERR_INVALID_DATA;

            fprintf(file, "%d\n", MY_IN);
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
        else if (strcmp(commandName, "JMP") == 0) {

            error = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);

            if (error != ERR_OK) return error;

            int address = findLabel(asmProgramData->labels, argument);

            if (address == -1) {
                printf("Unknown label: %s\n", argument);
                FCLOSE(file);
                return ERR_NOT_FOUND;
            }

            fprintf(file, "%d %d\n", MY_JMP, address);
        }
        else if (strcmp(commandName, "CALL") == 0) {

            error = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);
            if (error != ERR_OK) return error;

            int address = findLabel(asmProgramData->labels, argument);

            if (address == -1) {
                printf("Unknown label: %s\n", argument);
                FCLOSE(file);
                return ERR_NOT_FOUND;
            }

            fprintf(file, "%d %d\n", MY_CALL, address);
        }

        else if (strcmp(commandName, "RET") == 0) {

            fprintf(file, "%d\n", MY_RET);
        }
        else if (commandCondition (commandName)) {

            error = checkAsmOperation(sscanfResult, file, 2, __func__, __LINE__);

            if (error != ERR_OK) return error;

            int address = findLabel(asmProgramData->labels, argument);

            if (address == -1) {
                printf("Unknown label: %s\n", argument);
                FCLOSE(file);
                return ERR_NOT_FOUND;
            }

            int command = 0;

            if (strcmp(commandName, "JA")  == 0) command = MY_JA;
            else if (strcmp(commandName, "JB")  == 0) command = MY_JB;
            else if (strcmp(commandName, "JAE") == 0) command = MY_JAE;
            else if (strcmp(commandName, "JBE") == 0) command = MY_JBE;
            else if (strcmp(commandName, "JE")  == 0) command = MY_JE;
            else if (strcmp(commandName, "JNE") == 0) command = MY_JNE;
            else if (strcmp(commandName, "JT") == 0)  command = MY_JT;

            fprintf(file, "%d %d\n", command, address);
        }
        else {
            printf("Unknown command: %s\n", commandName);

            FCLOSE(file);
            return ERR_INVALID_DATA;
        }
    }

    FCLOSE(file);
    return ERR_OK;
}

bool commandCondition (const char* commandName){
    return strcmp(commandName, "JA")  == 0 ||
         strcmp(commandName, "JB")  == 0 ||
         strcmp(commandName, "JAE") == 0 ||
         strcmp(commandName, "JBE") == 0 ||
         strcmp(commandName, "JE")  == 0 ||
         strcmp(commandName, "JNE") == 0;
}
void removeAsmComment(char* line) {

    if (line == NULL) return;

    char* comment = strchr(line, ';');

    if (comment != NULL) {
        *comment = '\0';
    }
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

    if (asmData == NULL || asmData->labels == NULL) return ERR_INVALID_ARGUMENT;

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

    if (findLabel(labels, name) != -1) {
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

    if (labels == NULL || name == NULL) {
        return -1;
    }

    for (size_t i = 0; i < labels->labelCount; i++) {

        if (strcmp(labels->labelArr[i].labelName, name) == 0) {
            return labels->labelArr[i].labelValue;
        }
    }

    return -1;
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
