#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include <stdio.h>

VM vm;

// Resets the stack
static void resetStack(){
    vm.stackTop = vm.stack;
}

// New stack, start of VM
void initVM(){
    resetStack();
}

void freeVM(){

}

// Pushes a value into the stack
void push (Value value) {
    *vm .stackTop = value;
    vm.stackTop++;
}

// Pop the recently pushed item into the stack
Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}

static InterpretResult run() {
printf("\nwelcome to this whole new world of compilers\n\n");
// Some imp funcs 
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)\
    do {\
        double b = pop();\
        double a = pop();\
        push(a op b);\
    } while (false)
// Executing instructions
#ifndef DEBUG_TRACE_EXECUTION
    printf("    ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++){
        printf("[ ");
        printValue(*slot);
        printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    for (;;){
        uint8_t instruction;
        switch (instruction=READ_BYTE())
        {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NEGATE:   push(-pop());break;
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;

            case OP_RETURN:{
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        
        default:
            return INTERPRET_RUNTIME_ERROR;
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source){
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}