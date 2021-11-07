#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

VM vm;

// Resets the stack
static void resetStack(){
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
       CallFrame* frame = &vm.frames[i];
       ObjFunction* function = frame->function;
       size_t instruction = frame->ip - function->chunk.code - 1;
       fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
       if (function->name == NULL){
           fprintf(stderr, "script\n");
       } else {
           fprintf(stderr, "%s()\n", function->name->chars);
       }
    }
    resetStack();
}

static void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

/* 
    clock -> returns a double value
    lmao -> returns number value 69
    int -> converts int / str / bool to int
    str -> converts str / int / bool to str
    bool -> converts str / int / bool to bool
    
*/

int Error_Counter = 0;
int realArgCount = 0;
Value error;
static Value reportError(int realArgCount_, int errCount){
    error.type = VAL_ERROR;
    Error_Counter = errCount;
    realArgCount = realArgCount_;
    return error;
}

static Value clockNative(int argCount, Value* args) {
    int clockArgCount = 0;
    if (argCount != clockArgCount)return reportError(clockArgCount, 1);
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value lmao(int argCount, Value* args){
    int clockArgCount = 0;
    if (argCount != clockArgCount)return reportError(clockArgCount, 1);
    return NUMBER_VAL(69);
}


static Value valint(int argCount, Value* args){
    int clockArgCount = 1;
    if (argCount != clockArgCount)return reportError(clockArgCount, 1);
    Value convert = args[0];
    
    if (IS_OBJ(convert) && IS_STRING(convert)){
        ObjString* string = AS_STRING(convert);
        double result;
        char* eptr;

        result = strtod(string->chars, &eptr);
        return NUMBER_VAL(result);
    }
    else if (IS_NUMBER(convert))return convert;
    else if (IS_BOOL(convert))return NUMBER_VAL(convert.as.boolean?1:0);
    
}

static Value valstr(int argCount, Value* args){
    int clockArgCount = 1;
    if (argCount != clockArgCount){
        
    }
    
}

// New stack, start of VM
void initVM(){
    resetStack();
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);

    defineNative("clock", clockNative);
    defineNative("lmao", lmao);
    defineNative("int", valint);
    defineNative("str", valstr);
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

static bool call(ObjFunction* function, int argCount){
    if (argCount != function->arity){
        runtimeError("Expected %d arguments but got %d", function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX){
        runtimeError("Stack Overflow.");
        return false;
    }
    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount){
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_FUNCTION:
                return call(AS_FUNCTION(callee), argCount);
            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                if (IS_ERROR(result)){

                    switch (Error_Counter)
                    {
                    case 1:
                        runtimeError("Expected %d arguments but got %d", realArgCount, argCount);
                        break;
                    
                    case 2:
                        // Failed int conversion
                        runtimeError("Failed to convert to int");
                        break;

                    case 3:
                        // Failed str conversion
                        runtimeError("Failed to convert to str");
                        break;
                    
                    default:
                        break;
                    }
                    return false;
                    
                }
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break; // Invalid object-type call
        }
    }
    runtimeError("Can only call functions and classes");
    return false;
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void mulString(bool swap){
    ObjString*a;
    double b;
    if (!swap){
        a = AS_STRING(pop());
        b = AS_NUMBER(pop());
    } else {
        b = AS_NUMBER(pop());
        a = AS_STRING(pop());
    }

    int length = a->length * b;

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

    CallFrame* frame = &vm.frames[vm.frameCount - 1];

    // Some imp funcs 
    #define READ_BYTE() (*frame->ip++)

    #define READ_SHORT() \
        (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    #define READ_CONSTANT() \
        (frame->function->chunk.constants.values[READ_BYTE()])
    
    #define READ_SHORTER() \
        (frame->ip += 1, (uint8_t)(frame->ip[-1]))
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
        disassembleInstruction(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.code));
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

            case OP_CONSTANT_LONG: {
                // do something
                // Value constant = READ_CONSTANT();
                break;
            }

            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }


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
                push(frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
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
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
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
                Value result = pop();
                vm.frameCount--;
                if (vm.frameCount == 0){
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
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
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    call(function, 0);

    return run();
}
