# Virtual processor

The actual written virtual processor.
It was written in C.

## Author: Ekaterina Kraeva
## Mentor: Pavel Tarakanov

## Compilation and launch

### Compilation without DEBUG
```
g++ -g -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE mainVirtualPr.cpp -o virtual-processor.exe
```

### Compilation with DEBUG
```
g++ -g -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -DCPU_DEBUG -DSTACK_DEBUG=7 mainVirtualPr.cpp -o virtual-processor.exe
```

### Launch
```
./virtual-processor.exe asm-program.txt exe-file.txt
```

## Project parts

- Assembler — translates assembly instructions into executable numeric commands

- Disassembler — translates numeric commands back into assembly instructions

- Virtual CPU — executes instructions using a stack and registers

- Stack — a dynamic stack with debugging and memory protection

- Debugging system — provides CPU and stack dumps to help detect errors


## Project Structure
```
├── mainVirtualPr.cpp
├── virtual-processor.h
├── assembler.cpp
├── disassembler.cpp
├── processor.cpp
├── math-functions.cpp
├── extra-functions.cpp
├── used-files
│   ├── asm-program.txt
│   └── exe-program.txt
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
PUSH RAX
PUSH 10
JA :end

PUSH RAX
PUSH RAX
MUL
OUT

PUSH RAX
PUSH 1
ADD
POP RAX

JMP :next

:end
HLT

```

### Result

```
RESULT: 1
A new iteration of the cycle

RESULT: 4
A new iteration of the cycle

RESULT: 9
A new iteration of the cycle

RESULT: 16
A new iteration of the cycle

RESULT: 25
A new iteration of the cycle

RESULT: 36
A new iteration of the cycle

RESULT: 49
A new iteration of the cycle

RESULT: 64
A new iteration of the cycle

RESULT: 81
A new iteration of the cycle

RESULT: 100
A new iteration of the cycle

Program ended
CPU was destroyed
```
## Assembler
He assembler translates text instructions into numeric opcodes.

### ExeFile
```
1 1
23 0
22 0
1 10
15 14
22 0
22 0
4
12
22 0
1 1
2
23 0
14 2
13
```

## Virtual CPU

The virtual CPU reads the generated numeric instructions and executes them sequentially.

The CPU contains:
- An instruction pointer (IP).
- Four registers.
- A dynamic stack.
- A program instruction buffer.

## Stack

Stack is used from https://github.com/KatiaWeb12/Working-with-the-stack.git

## Debugging

Implemented a function `CPUDump()` that allows you to track the behavior of the processor during debugging. Also, the current processor action is highlighted in red. When switching to a new iteration of the loop, there is a message about this.

```
CPUInfo 'CPUData'[007CFAC0] created by mainTest() at mainVirtualPr.cpp:123
{
        textLength = 79
        stringsCount = 15
        textOfExeProgram[0147CA38]: 1 1 |  23 0 |  22 0 |  1 10 |  15 14 |  22 0 |  22 0 |  4 |  12 |  22 0 |  1 1 |  2 |  23 0 |  14 2 |  13 |
        registers[007CFAD4]{
                *[0] = 0(POISON)
                *[1] = 0(POISON)
                *[2] = 0(POISON)
                *[3] = 0(POISON)
        }



stack_t 'stk'[007CFAF8] created by CPULoad() at processor.cpp:12
{
        leftStructCanary = 8BADF00D
        leftDataCanary   = ABADBABE

        capacity = 3
        size = 1
        data[0147CA9B]{
                *[0] = 1
                *[1] = NaN(POISON)
                *[2] = NaN(POISON)
        }

        rightDataCanary   = B16B00B5
        rightStructCanary = 50FFC001
}
```
