# LC-3
A virtual machine that emulates LC-3 architecture and an assembler for the LC-3 assembly language.

The implementation of the virtual machine was based on Justin Meiners' [guide](https://www.jmeiners.com/lc3-vm/).

## Compilation
### Virtual Machine
    g++ main.cc CPU.cc platform.cc -o main -std=c++17 -Wall
    gcc main.cc CPU.cc platform.cc -o main -std=c++17 -Wall -lstdc++ -lm

### Assembler
    g++ main.cc Assembler.cc Command.cc -o main -std=c++17 -Wall
    gcc main.cc Assembler.cc Command.cc -o main -std=c++17 -Wall -lstdc++

## Usage
### Virtual Machine
    usage: ./main bin1 [bin2 ...]
           bin1, bin2, etc.: path to an assembled LC-3 program

### Assembler
    usage: ./main a [b ...]
           a, b, etc.: path to an LC-3 assembly program
