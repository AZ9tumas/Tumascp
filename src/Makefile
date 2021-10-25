tumascp: chunk.o main.o memory.o debug.o value.o vm.o scanner.o compiler.o object.o table.o
	gcc chunk.o main.o memory.o debug.o value.o vm.o scanner.o compiler.o object.o table.o -o tumascp
	rm *.o

debug.o: debug.h debug.c
	gcc -c debug.c

chunk.o: chunk.h chunk.c
	gcc -c chunk.c

main.o: main.c
	gcc -c main.c

memory.o: memory.c memory.h
	gcc -c memory.c

value.o: value.c value.h
	gcc -c value.c

vm.o: vm.c vm.h
	gcc -c vm.c

scanner.o: scanner.c scanner.h
	gcc -c scanner.c

compiler.o: compiler.c compiler.h
	gcc -c compiler.c

object.o: object.c object.h
	gcc -c object.c

table.o: table.c table.h
	gcc -c table.c