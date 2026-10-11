// Constants
const size_t MAX_PATH_LENGTH = 300;
const size_t MAX_COMMAND_LENGTH = 300;
const size_t START_LABEL_COUNT = 5;

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
    MY_POP_REG = 23,
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

struct labelItem {
    char* labelName;
    int labelValue;
};

struct label {
    size_t labelCapacity;
    size_t labelCount;
    labelItem* labelArr;
};

struct asmProgramInfo {
    size_t textLength;
    size_t stringsCount;
    char* textOfAsmProgram;
    char** programLines;

    label* labels;
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

// Assembler

ErrorCode asmLoad(asmProgramInfo* asmData, char* asmProgramFile);
ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData);
ErrorCode readAsmTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData);
ErrorCode recordPtrStringsForAsm(struct asmProgramInfo* asmProgramData);
ErrorCode assembler(char* fileName, struct asmProgramInfo* asmProgramData);
bool commandCondition (const char* commandName);

//Labels

ErrorCode loadLabels(label* labels);
ErrorCode addLabel(label* labels, char* name, int address);
int findLabel(const label* labels, const char* name);
ErrorCode collectLabels(asmProgramInfo* asmData);
ErrorCode resizeLabels(label* labels, int direction);


// Disassembler

ErrorCode disassembler(files* usedFiles);

// Processor

ErrorCode readExeProgram(const char* fileName, struct CPUInfo* CPUData);
ErrorCode readExeTextIntoSingleBuffer(const char* fileName, struct CPUInfo* CPUData);
ErrorCode recordPtrStringsForExe(struct CPUInfo* CPUData);
ErrorCode CPUexecute(char* fileName, CPUInfo* CPUData);
ErrorCode CPULoad(CPUInfo* CPUData, stack_t* stk, char* exeProgramFile);
ErrorCode cleanRegister(CPUInfo* CPUData);
ErrorCode CPUDestroy(CPUInfo* CPUData);

// Extra functions

ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount);
ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line);
ErrorCode printErrorIntoConsole(struct debugLog_t* debugLogInfo);
ErrorCode consoleProgramOutput(struct asmProgramInfo* asmProgramData);

void setFileNames(struct files* usedFiles, const size_t maxPathLength, int argc, char* argv[]);
void cancelBuffering();
size_t calculateStringsCount(const char* text);

int getWeekday();
bool ifTuesday();

// Math functions

ErrorCode addCommand(struct stack_t* stack);
ErrorCode subCommand(struct stack_t* stack);
ErrorCode mulCommand(struct stack_t* stack);
ErrorCode divCommand(struct stack_t* stack);
ErrorCode squareCommand(struct stack_t* stack);
ErrorCode sinCommand(struct stack_t* stack);
ErrorCode cosCommand(struct stack_t* stack);
ErrorCode tgCommand(struct stack_t* stack);
ErrorCode ctgCommand(struct stack_t* stack);
double degreesToRadians(double degrees);


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


#define FCLOSE(file)                                      \
    do {                                                  \
        if (fclose(file) != 0) {                          \
            printf("Warning: the file wasn't closed\n");  \
        }                                                 \
    } while (0)                                           \



