#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "time.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void repl() {
    char line[1024];
    printf("Tumascp: Version 0.1\tMade by-> AZ9tumas\n");
    printf("Type .exit to Exit.\n");
    for (;;){
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        
        if (line[0]=='.'&&line[1]=='e'&&line[2]=='x'&&line[3]=='i'&&line[4]=='t')return;

        clock_t start_t = clock();
        interpret(line);
        clock_t end_t = clock();

        printf("%ld %ld\n", start_t, end_t);
        
        clock_t total_t = end_t - start_t;
        total_t = (double)total_t;
        printf("Execution time taken: %ld\n", total_t);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s \".\n", path);
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
    InterpretResult result = interpret(source);
    free(source); 

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]){
    // Get the Virtual Machine ready!
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
    printf("Exit\n");
    return 0;
}
