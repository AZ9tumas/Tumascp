#ifndef tumascp_compiler_h
#define tumascp_compiler_h

#include "vm.h"
#include "object.h"

bool compile(const char* source, Chunk* chunk);

#endif