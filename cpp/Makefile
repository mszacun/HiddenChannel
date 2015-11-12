FLAGS=-std=c++11

all: Models.o main.o
	g++ $(FLAGS) Models.o main.o -o symulator

Models.o: Models.cpp Models.h
	g++ -c $(FLAGS) Models.cpp

main.o: main.cpp
	g++ -c $(FLAGS) main.cpp
