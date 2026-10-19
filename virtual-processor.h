// Constants
const size_t MAX_PATH_LENGTH = 300;
const size_t MAX_COMMAND_LENGTH = 300;
const size_t COMMANDS_COUNT = 25;
const size_t START_LABEL_COUNT = 5;
const size_t RAM_CAPACITY = 100;
const int    MAX_CALL_DEPTH = 256;

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

    MY_IN = 12,
    MY_OUT = 13,
    MY_HLT = 14,

    MY_JMP = 15,

    MY_JA = 16,
    MY_JB = 17,
    MY_JAE = 18,
    MY_JBE = 19,
    MY_JE = 20,
    MY_JNE = 21,
    MY_JT = 22,

    MY_CALL = 23,
    MY_RET = 24,
    MY_POP = 25
} asmCommand;

enum Register {
    RAX = 0,
    RBX = 1,
    RCX = 2,
    RDX = 3,

    REGISTER_COUNT = 4
};

enum ArgFlags {
    ARG_IMMED_VALUE = 1 << 0,  // 001 = 1
    ARG_REG         = 1 << 1,  // 010 = 2
    ARG_MEM         = 1 << 2   // 100 = 4
};

struct files {
        char path[MAX_PATH_LENGTH];
        char asmProgramFile[MAX_PATH_LENGTH];
        char exeFile[MAX_PATH_LENGTH];
        char binFile[MAX_PATH_LENGTH];
};

#ifdef CPU_DEBUG

struct debugCPU_t {
    const char* name;
    const char* file;
    const char* function;
    size_t line;
};
#endif

struct error_debug {
    const char* time;
    const char* file;
    const char* function;
    size_t line;
    const char* message;
    ErrorCode errCode;
};

struct commandHash_t {
    const char* commandName;
    hash_t hash;
};

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

struct binaryInstruction {
    int32_t opcode;
    int32_t flags;
    int32_t reg;
    int32_t imm;
};

struct CPUInfo {

    size_t stringsCount;
    binaryInstruction* program;

    stack_t* stack;

    size_t IP;
    int registers[REGISTER_COUNT];
    int RAM[RAM_CAPACITY];

    stack_t returnAddresses;

    #ifdef CPU_DEBUG
        debugCPU_t debugInfo;
    #endif
};

// Prototypes

// Assembler

// Assembler-work
ErrorCode asmLoad(asmProgramInfo* asmData, char* asmProgramFile);
ErrorCode readAsmProgram(const char* fileName, asmProgramInfo* asmProgramData);
ErrorCode readAsmTextIntoSingleBuffer(const char* fileName, struct asmProgramInfo* asmProgramData);
void recordPtrStringsForAsm(struct asmProgramInfo* asmProgramData);
ErrorCode assembler(const char* exeFileName, const char* binFileName, struct asmProgramInfo* asmProgramData);
bool commandCondition (int commandIndex);
void removeAsmComment(char* line);
ErrorCode parsePushPopArguments(const char* arg, int* flags, int* reg, int* imm);
ErrorCode writeCommand(FILE* exeFile, FILE* binFile,
                       int opcode, int flags, int reg, int imm,
                       int argsCount);

// Labels
ErrorCode loadLabels(label* labels);
ErrorCode addLabel(label* labels, char* name, int address);
int findLabel(const label* labels, const char* name);
ErrorCode collectLabels(asmProgramInfo* asmData);
ErrorCode resizeLabels(label* labels, int direction);

// RegisterIndex
int getRegisterIndex(const char* name);

// Hashing commands
void formCommandsArr(commandHash_t* commandsArr);
void createCommandHashEl(commandHash_t* commandsArr, const char* command, size_t commandIndex);
hash_t hashing(const char* str);

// Disassembler

ErrorCode disassembler(files* usedFiles);

// Processor

ErrorCode readExeProgram(const char* fileName, struct CPUInfo* CPUData);
ErrorCode readExeTextIntoSingleBuffer(const char* fileName, struct CPUInfo* CPUData);
ErrorCode recordPtrStringsForExe(struct CPUInfo* CPUData);
ErrorCode readBinaryProgram(const char* fileName, CPUInfo* CPUData);
ErrorCode CPUexecute(CPUInfo* CPUData);
ErrorCode CPULoad(CPUInfo* CPUData, stack_t* stk, char* exeProgramFile);
ErrorCode cleanRegister(CPUInfo* CPUData);
void CPUDestroy(CPUInfo* CPUData);
ErrorCode consoleInput(CPUInfo* CPUData);
bool jumpType(int el1, int el2, int command);

// Extra functions

ErrorCode checkStackBeforeOperation(stack_t* stack, size_t argCount);
ErrorCode checkAsmOperation(size_t sscanfResult, FILE* file, size_t operationArgsCount, const char* function, const int line);
ErrorCode printErrorIntoConsole(struct error_debug* debugLogInfo);
ErrorCode consoleProgramOutput(struct asmProgramInfo* asmProgramData);

ErrorCode setFileNames(struct files* usedFiles, const size_t maxPathLength, int argc, char* argv[]);
ErrorCode setBinFilename(char* exeFile, const size_t maxPathLength, int argc, char* argv[]);
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


#define ERROR_STRUCT_FORMAT(ptr, funcName, lineNum, msgValue, codeValue) \
    do {                                                                 \
        char timeString[32] = {};                                 \
        (ptr)->time = getOperationTime(timeString, sizeof(timeString));  \
        (ptr)->file = __FILE__;                                          \
        (ptr)->function = (funcName);                                    \
        (ptr)->line = (lineNum);                                         \
        (ptr)->message = (msgValue);                                     \
        (ptr)->errCode = (codeValue);                                    \
    } while (0)
