FLAGS=-std=c++11 -ggdb

all: Models.o main.o
	g++ $(FLAGS) Models.o main.o -o symulator

Models.o: Models.cpp Models.h
	g++ -c $(FLAGS) Models.cpp

main.o: main.cpp
	g++ -c $(FLAGS) main.cpp

clean:
	rm *.o
	rm symulator
