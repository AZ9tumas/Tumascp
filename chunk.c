#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// Making a proper chunk with 0 instructions
void initChunk(Chunk* chunk){
    chunk->capacity = 0;
    chunk->count = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

// Updates the current chunk by added the new 'byte'
void writeChunk(Chunk* chunk, uint8_t byte, int line){

    // Increases the capacity if it's less than the total count (including the new byte)
    // Updates line number array
    if (chunk->capacity < chunk->count + 1){
        int OldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(OldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, OldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, OldCapacity, chunk->capacity);
    }

    // Final update to 'count', 'line' and 'code'
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(Chunk* chunk, Value value){
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void freeChunk(Chunk* chunk){
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}