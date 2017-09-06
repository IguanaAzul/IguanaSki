TARGET = main
LIBS = -lglut -lGL -lGLU -lm

all: default
    
default: main

main.o: main.c
	gcc -c main.c -o main.o
	
glm.o: glm.c
	gcc -c glm.c -o glm.o

main: main.o glm.o
	gcc main.o glm.o -o main $(LIBS)

run: main
	./main

clean:
	-rm -f main.o glm.o
	-rm -f main
