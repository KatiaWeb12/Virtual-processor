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
#include "assembler.cpp"
#include "disassembler.cpp"
#include "processor.cpp"

void disassemblerTest(files* usedFiles);
ErrorCode mainTest(files* usedFiles);


int main(int argc, char* argv[]){

    cancelBuffering();

    struct files usedFiles = {};
    setFileNames(&usedFiles, MAX_PATH_LENGTH, argc, argv);

    mainTest(&usedFiles);

    return 0;
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
            printf(COLOR_RED "%s" COLOR_RESET " | ", CPUData->programLines[i]);
        }
        else {
            printf(" %s | ", CPUData->programLines[i]);
        }
    }

    putchar('\n');

    printf("\tregisters[%p]{    \n", CPUData->registers);

    for(size_t i = 0; i < REGISTER_COUNT; i++){

        if(CPUData->registers[i] == 0){
            printf("\t\t*[%u] = " COLOR_MAGENTA "%d(POISON)" COLOR_RESET "\n", i, 0);
        }
        else{
            printf("\t\t*[%u] = " PRINT_ELEM_FORMAT "\n", i, CPUData->registers[i]);
        }
    }

    printf("\t}\n\n");
    putchar('\n');

    STACK_DUMP(CPUData->stack);

    printf("}\n");

    getchar();

    return ERR_OK;
}
#endif

void disassemblerTest(files* usedFiles){

    disassembler(usedFiles);

}

ErrorCode mainTest(files* usedFiles){

    // Assembler
    struct asmProgramInfo asmProgramData = {};

    ErrorCode error = asmLoad(&asmProgramData, usedFiles->asmProgramFile);
    if (error != ERR_OK) return error;

    consoleProgramOutput(&asmProgramData);

    error = assembler(usedFiles->exeFile, &asmProgramData);
    if (error != ERR_OK) return error;

    // Processor
    struct CPUInfo CPUData = {};
    struct stack_t stk = {};

    CPULoad(&CPUData, &stk, usedFiles->exeFile);

    #ifdef CPU_DEBUG
        CPUData.debugInfo = {
            "CPUData",
            __FILE__,
            __func__,
            __LINE__
        };
    #endif

    CPUexecute(usedFiles->exeFile, &CPUData);

    CPUDestroy(&CPUData);

    return ERR_OK;

}


