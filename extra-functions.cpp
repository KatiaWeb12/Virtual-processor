#include <time.h>

#include "../universal-features/error.h"

// Error-processing and debugging

ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount){

    if(stack == NULL) return ERR_INVALID_ARGUMENT;

    if(stack->size < argCount) return ERR_INVALID_DATA;

    return ERR_OK;
}

ErrorCode printErrorIntoConsole(struct error_debug* debugLogInfo) {

    if(debugLogInfo == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    printf("[%s] Function '%s' was completed with Error %d in file '%s' in line %d \n",
            debugLogInfo->time,
            debugLogInfo->function,
            debugLogInfo->errCode,
            debugLogInfo->file,
            debugLogInfo->line);

    printf("ErrorMessage: %s \n", debugLogInfo->message);

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

    assert(text);
    assert(*text != '\0');

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

ErrorCode setFileNames(struct files* usedFiles, const size_t maxPathLength, int argc, char* argv[]){

    if(argc != 3){

        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Input-Error", ERR_INVALID_ARGUMENT);
        printErrorIntoConsole(&debugLogInfo);

        printf("Type: input file, output file.\nPaths must be no longer than %d characters.\n", maxPathLength);

        return ERR_INVALID_ARGUMENT;
    }

    const char* PATH = "./used-files/";
    sprintf(usedFiles->path, "%s", PATH);

    sprintf(usedFiles->asmProgramFile, "%s%s", PATH, argv[1]);

    sprintf(usedFiles->exeFile, "%s%s", PATH, argv[2]);

    putchar('\n');

    printf("Path:        [%s]\n", usedFiles->path);
    printf("Input file:  [%s]\n", usedFiles->asmProgramFile);
    printf("Result file: [%s]\n", usedFiles->exeFile);

    putchar('\n');

    return ERR_OK;
}

ErrorCode setExeFilename(char* exeFile, const size_t maxPathLength, int argc, char* argv[]){

    if(argc != 2){

        error_debug debugLogInfo = {};
        ERROR_STRUCT_FORMAT(&debugLogInfo, __func__, __LINE__, "Input-Error", ERR_INVALID_ARGUMENT);
        printErrorIntoConsole(&debugLogInfo);

        printf("Type: input file, output file.\nPaths must be no longer than %d characters.\n", maxPathLength);

        return ERR_INVALID_ARGUMENT;
    }

    const char* PATH = "./used-files/";
    sprintf(exeFile, "%s%s", PATH, argv[1]);

    putchar('\n');

    printf("Exe-file: [%s]\n", exeFile);

    putchar('\n');

    return ERR_OK;
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
