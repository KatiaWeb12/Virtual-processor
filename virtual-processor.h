#include "../universal-features/error.h"
#include "../Task5 stack/stack.h"

// Constants
const size_t MAX_PATH_LENGTH = 300;
const size_t MAX_COMMAND_LENGTH = 300;

// Types
typedef enum {
    MY_PUSH = 1,
    MY_ADD = 2,
    MY_SUB = 3,
    MY_MUL = 4,
    MY_DIV = 5,
    MY_SQUARE = 6,
    MY_SIN = 7,
    MY_COS = 8,
    MY_TG = 9,
    MY_CTG = 10,
    MY_BREAK = 11,
    MY_OUT = 12,
    MY_HLT = 13
} asmCommand;

struct files {
        char path[MAX_PATH_LENGTH];
        char asmProgramFile[MAX_PATH_LENGTH];
        char exeFile[MAX_PATH_LENGTH];
    };

struct asmProgramInfo {
    size_t textLength;
    size_t stringsCount;
    char* textOfAsmProgram;
    char** programLines;
};

// Prototypes
ErrorCode readTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData);
ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData);
ErrorCode recordPtrStrings(struct asmProgramInfo* asmProgramData);
ErrorCode formExeFile(char* fileName, struct asmProgramInfo* asmProgramData);
ErrorCode accomplishmentExeFile(char* fileName, stack_t* stack);
ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount);
size_t calculateStringsCount(const char* text);
void setFileNames(struct files* usedFiles, const size_t maxPathLength, int argc, char* argv[]);
void cancelBuffering();
ErrorCode consoleProgramOutput(struct asmProgramInfo* asmProgramData);
double degreesToRadians(double degrees);
ErrorCode openFile(char* fileName, const char mode);

ErrorCode addCommand(struct stack_t* stack);
ErrorCode subCommand(struct stack_t* stack);
ErrorCode mulCommand(struct stack_t* stack);
ErrorCode divCommand(struct stack_t* stack);
ErrorCode squareCommand(struct stack_t* stack);
ErrorCode sinCommand(struct stack_t* stack);
ErrorCode cosCommand(struct stack_t* stack);
ErrorCode tgCommand(struct stack_t* stack);
ErrorCode ctgCommand(struct stack_t* stack);
