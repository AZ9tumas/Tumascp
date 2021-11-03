#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

VM vm;

// Resets the stack
static void resetStack(){
    vm.stackTop = vm.stack;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

// New stack, start of VM
void initVM(){
    resetStack();
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
}

void freeVM(){
    freeTable(&vm.strings);
    freeObjects();
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

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void mulString(bool swap){
    ObjString*a;
    double b;
    printf("vars made\n");
    if (!swap){
        a = AS_STRING(pop());
        b = AS_NUMBER(pop());
    } else {
        b = AS_NUMBER(pop());
        a = AS_STRING(pop());
    }

    int length = a->length * b;
    printf("len: %d\n", length);

    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    
    int i = 1;
    for (i = 1; i<b; i++){
        memcpy(chars + (a->length * i), a->chars, a->length);
    }
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}

static void concatenate() {
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}

static InterpretResult run() {

// Some imp funcs 
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_SHORT() \
    (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8 | vm.ip[-1])))
#define READ_SHORTER() \
    (vm.ip += 1, (uint8_t)(vm.ip[-1]))
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
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

            case OP_CONSTANT_LONG:
                // do something
                // Value constant = READ_CONSTANT();


            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))){
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;

            case OP_POPC:
                uint8_t times = READ_SHORTER();
                for (int count = 0; count<times; count++)pop();
                break;

            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                tableDelete(&vm.globals, name); 
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm.stack[slot]); 
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm.stack[slot] = peek(0);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0)))vm.ip += offset;
                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD:
            {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: {

                if (IS_STRING(peek(0)) && IS_NUMBER(peek(1)) || IS_NUMBER(peek(0)) && IS_STRING(peek(1))) {
                    mulString(IS_NUMBER(peek(0)) && IS_STRING(peek(1)));
                }
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a*b));
                } else {
                    runtimeError("Operands must be two numbers or number and string.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;

            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }

            case OP_RETURN:{
                // Exit
                return INTERPRET_OK;
            }


        
        default:
            return INTERPRET_RUNTIME_ERROR;
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
#undef READ_SHORT
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
