#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

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
        long double result;
        char* endPointer;
        result = strtod(AS_STRING(convert)->chars, &endPointer);
        if (*endPointer == '\0')return NUMBER_VAL(result);
        return NIL_VAL;
    }
    else if (IS_NUMBER(convert))return convert;
    else if (IS_BOOL(convert))return NUMBER_VAL(convert.as.boolean?1:0);
    return NIL_VAL;
}

static int getDigits(long double number){
    int digits = 0;
    while (number != 0){
        number = trunc(number);
        digits++;
    }
    return digits;
}

Value tumascp_str(int argCount, Value* args){
    int realarg = 1;
    if (argCount != realarg)return reportError(realarg, 1);
    Value convert = args[0];
    if (IS_NUMBER(convert)){
        char final[1024];
        snprintf(final, 1024, "%g", AS_NUMBER(convert));
        int len = 0;
        for (int i = 0; i<1000; i++){
            if (final[i] == '\0'||final[i]==-1)break;
            len++;
        }
        return OBJ_VAL(copyString(final, len));
    } else if (IS_OBJ(convert)){
        if (IS_STRING(convert))return convert;
        else if (IS_FUNCTION(convert)){
            ObjFunction* func = AS_FUNCTION(convert);
            return OBJ_VAL(copyString(func->name->chars, func->name->length));
        }
        else if (IS_NATIVE(convert))return OBJ_VAL(copyString("inbuilt",8));
    } else if (IS_BOOL(convert)){
        if (isFalsey(convert))return OBJ_VAL(copyString("true",5));
        else return OBJ_VAL(copyString("false",6));
    } else if (IS_NIL(convert))return OBJ_VAL(copyString("nil", 4));
    return NIL_VAL;
}

Value tumascp_print(int argCount, Value* args){
    // max args: 255 (checked during compiling)
    
    for (int i = 0; i<argCount; i++){
        printValue(args[i]);
        printf(" ");
    }
    printf("\n");

    return NIL_VAL;
}

Value tumascp_bool(int argCount, Value* args){
    int realarg = 1;
    if (argCount != realarg)return reportError(realarg, 1);
    Value convert = args[0];

    if (IS_NUMBER(convert)){
        double number = AS_NUMBER(convert);
        return BOOL_VAL(number!=0);
    } else if (IS_OBJ(convert)&&IS_STRING(convert)){
        if (AS_STRING(convert)->chars)return BOOL_VAL(true);
        return BOOL_VAL(false);
    } else if (IS_BOOL(convert)){
        return convert;
    }
    return BOOL_VAL(false);
}

Value tumascp_type(int argCount, Value* args){
    int realarg = 1;
    if (argCount != realarg)return reportError(realarg, 1);
    Value convert = args[0];

    if (IS_NUMBER(convert))return OBJ_VAL(copyString("int", 4));
    else if (IS_FUNCTION(convert))return OBJ_VAL(copyString("function", 9));
    else if (IS_NATIVE(convert))return OBJ_VAL(copyString("native", 7));
    else if (IS_STRING(convert))return OBJ_VAL(copyString("string", 7));
    else if (IS_BOOL(convert))return OBJ_VAL(copyString("bool", 5));
    else if (IS_NIL(convert))return OBJ_VAL(copyString("nil", 4));
}

Value tumascp_input(int argCount, Value* args){
    int maxArg = 1;
    if (argCount > maxArg)return reportError(maxArg, 1);

    Value display = args[0];
    char input[1024];
    if (IS_OBJ(display) && argCount != 0){
        //if (!IS_STRING(display))return reportError(maxArg, 4);
        printValue(display);
    }
    if(!fgets(input, sizeof(input), stdin))return reportError(maxArg, 4);
    
    //printf("\n");

    int size = (int)strlen(input);

    ObjString* inputline = copyString(input, size>0?size-1:0);
    return OBJ_VAL(inputline);
    
}