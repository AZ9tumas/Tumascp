#include "common.h"
#include "vm.h"
#include "debug.h"
#include <stdio.h>

VM vm;

void initVM(){

}

void freeVM(){

}

static InterpretResult run() {

// Some imp funcs 
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// Executing instructions
#ifndef DEBUG_TRACE_EXECUTION
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    for (;;){
        uint8_t instruction;
        switch (instruction=READ_BYTE())
        {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break;
            }

            case OP_RETURN:{
                
                return INTERPRET_OK;
            }
        
        default:
            return INTERPRET_RUNTIME_ERROR;
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Chunk* chunk){
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}