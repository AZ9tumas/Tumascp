#ifndef tumascp_global_h
#define tumascp_global_h

#include "common.h"
#include "value.h"
#include "object.h"
#include "vm.h"

int getError_Counter();
int getrealArgCount();

// Inbuilt functions

Value tumascp_print(int argCount, Value* args);
Value tumascp_int(int argCount, Value* args);
Value tumascp_str(int argCount, Value* args);
Value tumascp_bool(int argCount, Value* args);
Value tumascp_clock(int argCount, Value* args);
Value tumascp_input(int argCount, Value* args);
Value tumascp_type(int argCount, Value* args);
Value tumascp_exit(int argCount, Value* args);

#endif