#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include <time.h>
#include <string.h>

int Error_Counter = 0;
int realArgCount = 0;
Value error;

static bool isDigit(char c){if(c=='.')return true;return c>='0'&&c<='9';}

static Value reportError(int realArgCount_, int errCount){
    error.type = VAL_ERROR;
    Error_Counter = errCount;
    realArgCount = realArgCount_;
    return error;
}

int getError_Counter(){
    return Error_Counter;
}

int getrealArgCount(){
    return realArgCount;
}

Value tumascp_clock(int argCount, Value* args) {
    int realarg = 0;
    if (argCount != realarg)return reportError(realarg, 1);
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value tumascp_int(int argCount, Value* args){
    int realarg = 1;
    if (argCount != realarg)return reportError(realarg, 1);
    Value convert = args[0];
    
    if (IS_OBJ(convert) && IS_STRING(convert)){
        ObjString* string = AS_STRING(convert);

        int count = 0;
        for (;;){
            if (string->chars[count] == '\0')break;
            if (!isDigit(string->chars[count]))return reportError(realarg, 2);
            count++;
        }

        double result;
        char* eptr;

        result = strtod(string->chars, &eptr);
        return NUMBER_VAL(result);
    }
    else if (IS_NUMBER(convert))return convert;
    else if (IS_BOOL(convert))return NUMBER_VAL(convert.as.boolean?1:0);
    
}

Value tumascp_str(int argCount, Value* args){
    int realarg = 1;
    if (argCount != realarg)return reportError(realarg, 1);
}

Value tumascp_input(int argCount, Value* args){
    int maxArg = 1;
    if (argCount > maxArg)return reportError(maxArg, 1);

    Value display = args[0];
    char input[1024];
    if (IS_OBJ(display)){
        //if (!IS_STRING(display))return reportError(maxArg, 4);
        printValue(display);
    }
    if(!fgets(input, sizeof(input), stdin))return reportError(maxArg, 4);
    
    //printf("\n");

    int size = (int)strlen(input);

    ObjString* inputline = copyString(input, size>0?size-1:0);
    return OBJ_VAL(inputline);
    
}