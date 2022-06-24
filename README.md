# LC-3-VM
A virtual machine that emulates LC-3 architecture and executes assembled LC-3 programs.

The implementation was based on Justin Meiners' [guide](https://www.jmeiners.com/lc3-vm/).

## Compilation
    g++ main.cc CPU.cc platform.cc -o main -std=c++17 -Wall
    gcc main.cc CPU.cc platform.cc -o main -std=c++17 -Wall -lstdc++ -lm

## Usage
    usage: ./main bin1 [bin2 ...]
           bin1, bin2, etc.: path to an assembled LC-3 program
