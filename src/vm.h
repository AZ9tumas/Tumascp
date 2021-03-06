#ifndef tumascp_vm_h
#define tumascp_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
  ObjClosure* closure;
  uint8_t* ip;
  Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    ObjUpvalue* openUpalues;
    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_EXIT
} InterpretResult;

extern VM vm;

// Imp VM functions
void initVM();
void freeVM();
InterpretResult interpret(const char* source, bool repl);

// Stack funcs
void push(Value value);
Value popn(int n);
Value pop();

#endif
