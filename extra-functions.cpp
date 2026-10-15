#include <time.h>

#include "../universal-features/error.h"

// Error-processing and debugging

ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount){

    if(stack == NULL) return ERR_INVALID_ARGUMENT;

    if(stack->size < argCount) return ERR_INVALID_DATA;

    return ERR_OK;
}

ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line){

    if (sscanfResult != operationArgsCount) {

        FCLOSE(file);

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

ErrorCode consoleProgramOutput(struct asmProgramInfo* asmProgramData){

    if(asmProgramData == NULL) return ERR_INVALID_ARGUMENT;

    printf("-----------------\nConsole ASM-program printing:\n");
    for(size_t i = 0; i < asmProgramData->stringsCount; i++){
        printf("%s \n", asmProgramData->programLines[i]);
    }
    printf("-----------------\n\n");
    return ERR_OK;
}

// Preparing functions

size_t calculateStringsCount(const char* text) {

    if (text == NULL || *text == '\0') {
        return 0;
    }

    size_t stringCount = 0;

    for (const char* ptr = text; *ptr != '\0'; ptr++) {
        if (*ptr == '\n') {
            stringCount++;
        }
    }

    if (text[strlen(text) - 1] != '\n') {
        stringCount++;
    }

    return stringCount;
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

// Time

int getWeekday(){

    time_t currentTime = time(NULL);
    struct tm* date = localtime(&currentTime);

    int weekday = date->tm_wday;

    return weekday;
}

bool ifTuesday(){
    return getWeekday() == 2;
}
