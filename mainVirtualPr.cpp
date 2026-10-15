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

    putchar('\n');

    struct files usedFiles = {};
    setFileNames(&usedFiles, MAX_PATH_LENGTH, argc, argv);

    putchar('\n');

    mainTest(&usedFiles);

    return 0;
}

void disassemblerTest(files* usedFiles){

    disassembler(usedFiles);

}

ErrorCode mainTest(files* usedFiles){

    // Assembler
    struct asmProgramInfo asmProgramData = {};

    ErrorCode error = asmLoad(&asmProgramData, usedFiles->asmProgramFile);
    if (error != ERR_OK) return error;
    #ifdef CPU_DEBUG
    consoleProgramOutput(&asmProgramData);
    #endif

    error = assembler(usedFiles->exeFile, &asmProgramData);
    if (error != ERR_OK) return error;

    // Processor
    struct CPUInfo CPUData = {};
    struct stack_t stk = {};

    error = CPULoad(&CPUData, &stk, usedFiles->exeFile);
    if (error != ERR_OK) return error;

    #ifdef CPU_DEBUG
        CPUData.debugInfo = {
            "CPUData",
            __FILE__,
            __func__,
            __LINE__
        };
    #endif

    CPUexecute(&CPUData);

    CPUDestroy(&CPUData);

    return ERR_OK;

}
