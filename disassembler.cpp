#include "../universal-features/error.h"

ErrorCode disassembler(files* usedFiles) {

    if (usedFiles == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    char* exeFileName = usedFiles->exeFile;
    char* asmFileName = usedFiles->asmProgramFile;

    FILE* exeFile = fopen(exeFileName, "r");

    if (exeFile == NULL) {
        return ERR_FILE_NOT_FOUND;
    }

    FILE* asmFile = fopen(asmFileName, "w");

    if (asmFile == NULL) {
        FCLOSE(exeFile);
        return ERR_FILE_NOT_FOUND;
    }

    int command = 0;
    int argument = 0;

    const char* registerNames[REGISTER_COUNT] = {
        "RAX",
        "RBX",
        "RCX",
        "RDX"
    };

    char line[MAX_COMMAND_LENGTH] = {};

    while (fgets(line, sizeof(line), exeFile) != NULL) {

        command = 0;
        argument = 0;

        int sscanfResult = sscanf(line, "%d %d", &command, &argument);

        if (sscanfResult < 1) {
            continue;
        }

        switch (command) {

            case MY_PUSH: {

                if (sscanfResult != 2) {
                    goto disassemblerError;
                }

                fprintf(asmFile, "PUSH %d\n", argument);
                break;
            }

            case MY_PUSH_REG: {

                if (sscanfResult != 2 || argument < 0 || argument >= REGISTER_COUNT) goto disassemblerError;

                fprintf(asmFile, "PUSH %s\n", registerNames[argument]);

                break;
            }

            case MY_POP_REG: {

                if (sscanfResult != 2 || argument < 0 || argument >= REGISTER_COUNT) goto disassemblerError;

                fprintf(asmFile, "POP %s\n", registerNames[argument]);
                break;
            }

            case MY_ADD: {
                fprintf(asmFile, "ADD\n");
                break;
            }

            case MY_SUB: {
                fprintf(asmFile, "SUB\n");
                break;
            }

            case MY_MUL: {
                fprintf(asmFile, "MUL\n");
                break;
            }

            case MY_DIV: {
                fprintf(asmFile, "DIV\n");
                break;
            }

            case MY_SQUARE: {
                fprintf(asmFile, "SQUARE\n");
                break;
            }

            case MY_SIN: {
                fprintf(asmFile, "SIN\n");
                break;
            }

            case MY_COS: {
                fprintf(asmFile, "COS\n");
                break;
            }

            case MY_TG: {
                fprintf(asmFile, "TG\n");
                break;
            }

            case MY_CTG: {
                fprintf(asmFile, "CTG\n");
                break;
            }

            case MY_BREAK: {
                fprintf(asmFile, "BREAK\n");
                break;
            }

            case MY_OUT: {
                fprintf(asmFile, "OUT\n");
                break;
            }

            case MY_HLT: {
                fprintf(asmFile, "HLT\n");
                break;
            }

            case MY_JMP: {

                if (sscanfResult != 2) {
                    goto disassemblerError;
                }

                fprintf(asmFile, "JMP :%d\n", argument);

                break;
            }
            case MY_JA:
            case MY_JB:
            case MY_JAE:
            case MY_JBE:
            case MY_JE:
            case MY_JNE:
            case MY_JT: {

                if (sscanfResult != 2) {
                    goto disassemblerError;
                }

                const char* commandName = NULL;

                switch (command) {

                    case MY_JA:  commandName = "JA";  break;
                    case MY_JB:  commandName = "JB";  break;
                    case MY_JAE: commandName = "JAE"; break;
                    case MY_JBE: commandName = "JBE"; break;
                    case MY_JE:  commandName = "JE";  break;
                    case MY_JNE: commandName = "JNE"; break;
                    case MY_JT:  commandName = "JT";  break;

                    default: break;
                }

                fprintf(asmFile, "%s :%d\n", commandName, argument);

                break;
            }

            case MY_CALL: {

                if (sscanfResult != 2) {
                    goto disassemblerError;
                }

                fprintf(asmFile, "CALL :%d\n", argument);
                break;
            }

            case MY_RET: {

                fprintf(asmFile, "RET\n");
                break;
            }

            default: {

                printf("Unknown command: %d\n", command);
                goto disassemblerError;
            }
        }
    }

    FCLOSE(exeFile);
    FCLOSE(asmFile);

    return ERR_OK;

    disassemblerError:

        FCLOSE(exeFile);
        FCLOSE(asmFile);

        return ERR_INVALID_DATA;
}
