#ifndef tumascp_global_h
#define tumascp_global_h

#include "common.h"
#include "value.h"
#include "object.h"

int getError_Counter();
int getrealArgCount();

// Inbuilt functions

Value tumascp_print(int argCount, Value* args);
Value tumascp_int(int argCount, Value* args);
Value tumascp_str(int argCount, Value* args);
Value tumascp_clock(int argCount, Value* args);
Value tumascp_input(int argCount, Value* args);

#endif