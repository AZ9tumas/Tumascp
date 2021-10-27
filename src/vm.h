#ifndef tumascp_vm_h
#define tumascp_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 512

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

// Imp VM functions
void initVM();
void freeVM();
InterpretResult interpret(const char* source);

// Stack funcs
void push(Value value);
Value pop();

#endif