#include <stdio.h>
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]){

    // Make the Virtual Machine
    initVM();

    printf("argc(int): %d \nargv(const char): %s\n\n", argc, *argv);
    // Creating a new chunk which accepts instructions
    Chunk chunk;
    initChunk(&chunk);

    // Testing constants by "hand-compiling"
    int constant = addConstant(&chunk, 69.42);
    writeChunk(&chunk, OP_CONSTANT,123);
    writeChunk(&chunk, constant,123);

    writeChunk(&chunk, OP_RETURN,123);

    // Debugging so humans can read and understand what's going on
    disassembleChunk(&chunk, "test chunk");
    interpret(&chunk);

    // Remove the Virtual Machine which contains chunks
    freeVM();

    // Remove chunk and get rid of instructions
    freeChunk(&chunk);

    return 0;
}