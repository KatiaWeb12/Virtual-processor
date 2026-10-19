# Virtual processor

The actual written virtual processor.
It was written in C.

## Author: Ekaterina Kraeva
## Mentor: Pavel Tarakanov

## Compilation and launch

### Compilation assembler
```
g++ -g -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE assembler.cpp -o assembler.exe
```
### Compilation processor without DEBUG
```
g++ -g -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE processor.cpp -o processor.exe
```

### Compilation processor with DEBUG
```
g++ -g -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -DCPU_DEBUG -DSTACK_DEBUG=7 processor.cpp -o processor.exe
```

### Launch
```
./assembler.exe quad-equation.txt exe-file.txt binFile.bin
./processor.exe binFile.bin
```

## Project parts

- Assembler — translates assembly instructions into executable numeric commands / binary commands

- Disassembler — translates bynary commands back into assembly instructions

- Virtual CPU — executes instructions using a stack and registers

- Stack — a dynamic stack with debugging and memory protection

- Debugging system — provides CPU and stack dumps to help detect errors


## Project Structure
```
Task 6 virtual processor/
    ├── assembler.cpp
    ├── disassembler.cpp
    ├── processor.cpp
    ├── virtual-processor.h
    ├── math-functions.cpp
    ├── extra-functions.cpp
    ├── used-files/...
    └── README.md
```

## Assembly Instructions

Arithmetic Operations

|Command|Description|
|:-|:-:|
|ADD|Adds two stack values|
|SUB|Subtracts two stack values|
|MUL|Multiplies two stack values|
|DIV|Divides two stack values|
|SQUARE|Calculates the square root of the top stack value|
|JMP :label|Unconditional jump|
|JA :label|First value > second value|
|JB :label|First value < second value|
|JAE :label|First value >= second value|
|JBE :label|First value <= second value|
|JE :label|First value == second value|
|JNE :label|First value != second value|
|JT :label|Jump if today is Tuesday|
|OUT|Pops and prints the top stack value|
|HLT|Stops program execution|
|BREAK|Intentionally corrupts the stack's right datacanary for testing|

## Example: Squares from 1 to 10
```
PUSH 1
POP RAX

:next
PUSH 10
PUSH RAX
JA :end

PUSH RAX
CALL :square
OUT

PUSH RAX
PUSH 1
ADD
POP RAX

JMP :next

:end
HLT

:square
POP RBX
PUSH RBX
PUSH RBX
MUL
RET


```

### Result

```
RESULT: 1
RESULT: 4
RESULT: 9
RESULT: 16
RESULT: 25
RESULT: 36
RESULT: 49
RESULT: 64
RESULT: 81
RESULT: 100

Program ended
CPU was destroyed
```
## Assembler
He assembler translates text instructions into numeric opcodes and into bynary format.

## Binary format
```
enum ArgFlags {
    ARG_IMMED_VALUE = 1 << 0,  // 001 = 1
    ARG_REG         = 1 << 1,  // 010 = 2
    ARG_MEM         = 1 << 2   // 100 = 4
};
```
```
struct binaryInstruction {
    int32_t opcode;
    int32_t flags;
    int32_t reg;
    int32_t imm;
};
```

### ExeFile
```
1 1 0 1
25 2 0 0
1 1 0 10
1 2 0 0
16 13
1 2 0 0
23 14
13
1 2 0 0
1 1 0 1
...
```

## Virtual CPU

The virtual CPU reads the generated numeric instructions and executes them sequentially.

The CPU contains:
- An instruction pointer (IP)
- Four registers
- A dynamic stack
- A program instruction buffer
- RAM

## Stack

Stack is used from https://github.com/KatiaWeb12/Working-with-the-stack.git

## Debugging

Implemented a function `CPUDump()` that allows you to track the behavior of the processor during debugging. Also, the current processor action is highlighted in red. When switching to a new iteration of the loop, there is a message about this.

## Realised assembler-code files

- ellipse.txt              Draw ellipse using RAM
- factorial.txt            Calculate factorial
- quad-equation.txt        Solves the quadratic equation
- squares-numbers1-10.txt  Counts squares of numbers from 1 to 10
