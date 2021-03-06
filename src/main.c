#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void repl() {
    char line[1024];
    printf("\n\t----------------------------------------\n");
    printf("\t| ==== [ TUMASCP Version: 0.2.1 ] ==== |\n");
    printf("\t----------------------------------------\n");
    printf("\t          BUILT BY AZ9TUMAS\n\n\n");
    printf("Tip: Use 'exit()' to exit the REPL\n\n");

    for (;;){
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        InterpretResult interpret_result =  interpret(line, true);
        if (interpret_result == INTERPRET_EXIT)return;
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        char* nothing = (char*)malloc(8);
        return nothing;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);

    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    if (strcmp(source, "")==true){
        free(source);
        printf("Empty file.\n");
        exit(60);
    }
    InterpretResult result = interpret(source, false);
    free(source); 

    if (result == INTERPRET_EXIT) exit(60);
    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]){
    initVM();

    if (argc == 1){
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: tumascp [path]\n");
        exit(64);
    }
    freeVM();
    return 0;
}
