#include "../universal-features/error.h"

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

ErrorCode CPUexecute(char* exeFile, CPUInfo* CPUData){

    FILE* file = fopen(exeFile, "r");
    if(file == NULL) return ERR_UNKNOWN;

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
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                addCommand(CPUData->stack);

                break;
            }

            case MY_SUB: {

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                subCommand(CPUData->stack);

                break;
            }

            case MY_MUL:{

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                mulCommand(CPUData->stack);

                break;
            }

            case MY_DIV: {

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                divCommand(CPUData->stack);

                break;
            }

            case MY_SQUARE: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                squareCommand(CPUData->stack);

                break;
            }

            case MY_SIN:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                sinCommand(CPUData->stack);

                break;
            }

            case MY_COS:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                cosCommand(CPUData->stack);

                break;
            }

            case MY_TG: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                tgCommand(CPUData->stack);

                break;
            }

            case MY_CTG: {

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                ctgCommand(CPUData->stack);

                break;
            }

            case MY_BREAK:{
                *getRightDataCanary(CPUData->stack) = 8;
                break;
            }

            case MY_OUT:{

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    printf("ERROR: not enough elements in stack\n");
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                stackElem_t value = 0;
                stackPop(CPUData->stack, &value);
                printf(COLOR_RED "RESULT: %d \n" COLOR_RESET, (stackElem_t)((double)(value) / 1000));

                break;
            }
            case MY_HLT: {

                printf("\nProgram ended\n");
                FCLOSE(file);
                return ERR_OK;
            }
            case MY_JMP: {

                if (sscanfResult != 2) {
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) {
                    FCLOSE(file);
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
                    FCLOSE(file);
                    return ERR_INVALID_ARGUMENT;
                }

                ErrorCode error = stackPush(CPUData->stack, CPUData->registers[arg]);
                if (error != ERR_OK) {
                    FCLOSE(file);
                    return error;
                }

                break;
            }
            case MY_POP_REG: {

                if (arg < 0 || arg >= REGISTER_COUNT) {
                    FCLOSE(file);
                    return ERR_INVALID_ARGUMENT;
                }

                if (checkStackBeforeOperation(CPUData->stack, 1) != ERR_OK) {
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                ErrorCode error = stackPop(CPUData->stack, &(CPUData->registers[arg]));

                if (error != ERR_OK) {
                    FCLOSE(file);
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
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) {
                    FCLOSE(file);
                    return ERR_OUT_OF_BOUNDS;
                }

                if (checkStackBeforeOperation(CPUData->stack, 2) != ERR_OK) {
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }

                stackElem_t el2 = 0;
                stackElem_t el1 = 0;

                stackPop(CPUData->stack, &el2);
                stackPop(CPUData->stack, &el1);

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

                if (jump) {
                    CPU_DUMP(CPUData, CPUData->IP);
                    CPUData->IP = (size_t)arg;
                    continue;
                }

                break;
            }
            case MY_JT: {

                if (sscanfResult != 2) {
                    FCLOSE(file);
                    return ERR_INVALID_DATA;
                }
                if (arg < 0 || (size_t)arg >= CPUData->stringsCount) {
                    FCLOSE(file);
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
            default:{

                printf("Unknown command: %d\n", command);
                FCLOSE(file);
                return ERR_INVALID_DATA;

            }
        }

        CPU_DUMP(CPUData, CPUData->IP);
        CPUData->IP++;

    }

    return ERR_INVALID_DATA;
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

    free(CPUData->textOfExeProgram);
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

int getRegisterIndex(const char* name) {

    if (strcmp(name, "RAX") == 0) return RAX;
    if (strcmp(name, "RBX") == 0) return RBX;
    if (strcmp(name, "RCX") == 0) return RCX;
    if (strcmp(name, "RDX") == 0) return RDX;

    return -1;
}


