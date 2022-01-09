#ifndef tumascp_chunk_h
#define tumascp_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_RETURN,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLOSE_UPVALUE,
    OP_DEFINE_GLOBAL,
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_ABS_JUMP,
    OP_PRINT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POPC,
    OP_POP
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    ValueArray constants;
    int* lines;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif
