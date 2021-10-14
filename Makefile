tumascp: chunk.o main.o memory.o debug.o value.o vm.o
	gcc chunk.o main.o memory.o debug.o value.o vm.o -o tumascp
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