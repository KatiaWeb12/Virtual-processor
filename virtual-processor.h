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
    MY_HLT = 13,

    MY_JMP = 14,
    MY_JA = 15,
    MY_JB = 16,
    MY_JAE = 17,
    MY_JBE = 18,
    MY_JE = 19,
    MY_JNE = 20,
    MY_JT = 21,

    MY_PUSH_REG = 22,
    MY_POP_REG = 23

} asmCommand;

enum Register {
    RAX = 0,
    RBX = 1,
    RCX = 2,
    RDX = 3,

    REGISTER_COUNT = 4
};

struct files {
        char path[MAX_PATH_LENGTH];
        char asmProgramFile[MAX_PATH_LENGTH];
        char exeFile[MAX_PATH_LENGTH];
};

#ifdef CPU_DEBUG

struct debugCPU_t {
    const char* name;
    const char* file;
    const char* function;
    size_t line;
};

#endif

struct asmProgramInfo {
    size_t textLength;
    size_t stringsCount;
    char* textOfAsmProgram;
    char** programLines;

    int labels;
};

struct CPUInfo {

    size_t textLength;
    size_t stringsCount;
    char* textOfExeProgram;
    char** programLines;

    stack_t* stack;

    int registers[REGISTER_COUNT];
    size_t IP;

    #ifdef CPU_DEBUG
        debugCPU_t debugInfo;
    #endif
};

// Prototypes
ErrorCode readAsmTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData);
ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData);
ErrorCode recordPtrStringsForAsm(struct asmProgramInfo* asmProgramData);
ErrorCode assembler(char* fileName, struct asmProgramInfo* asmProgramData);
ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount);
size_t calculateStringsCount(const char* text);
void setFileNames(struct files* usedFiles, const size_t maxPathLength, int argc, char* argv[]);
void cancelBuffering();
ErrorCode consoleProgramOutput(struct asmProgramInfo* asmProgramData);
double degreesToRadians(double degrees);
ErrorCode openFile(char* fileName, const char mode);
ErrorCode printErrorIntoConsole(struct debugLog_t* debugLogInfo);
ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line);

// Processor

ErrorCode readExeProgram(const char* fileName, struct CPUInfo* CPUData);
ErrorCode readExeTextIntoSingleBuffer(const char* fileName, struct CPUInfo* CPUData);
ErrorCode recordPtrStringsForExe(struct CPUInfo* CPUData);
ErrorCode CPUexecute(char* fileName, CPUInfo* CPUData);
ErrorCode CPULoad(CPUInfo* CPUData, stack_t* stk, char* exeProgramFile);
ErrorCode cleanRegister(CPUInfo* CPUData);
ErrorCode CPUDestroy(CPUInfo* CPUData);

int getRegisterIndex(const char* name);


#ifdef CPU_DEBUG
    ErrorCode CPUDump(const struct CPUInfo* CPUData, size_t currentLine);
#endif


//Macro
#ifdef CPU_DEBUG
    #define CPU_DUMP(cpu, line) CPUDump((cpu), (line))
#else
    #define CPU_DUMP(cpu, line) ((void)0)
#endif



