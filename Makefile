all: memgrind.o mymalloc.o
	gcc -g -Wall -Werror -fsanitize=address -o memgrind mymalloc.o memgrind.o
mymalloc.o: mymalloc.c mymalloc.h
	gcc -c -g mymalloc.c
memgrind.o: memgrind.c
	gcc -c -g memgrind.c
clean:
	rm memgrind *.o
