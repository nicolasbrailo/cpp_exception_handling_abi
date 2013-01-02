all: app

throw.o: throw.cpp throw.h
		g++ -c -o throw.o -O0 -ggdb throw.cpp

mycppabi.o: mycppabi.cpp
		g++ -c -o mycppabi.o -O0 -ggdb mycppabi.cpp

main.o: main.c
		gcc -c -o main.o -O0 -ggdb main.c

app: main.o throw.o mycppabi.o
		gcc main.o throw.o mycppabi.o -O0 -ggdb -o app

throw.gas: throw.cpp
		g++ -c throw.cpp -g -Wa,-adhls > throw.gas

throw.s: throw.cpp
		g++ -S throw.cpp

.PHONY: clean run
clean:
	rm -f main.o throw.o mycppabi.o app

run: app
		./app

