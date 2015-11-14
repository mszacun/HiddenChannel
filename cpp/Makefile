FLAGS=-std=c++11 -O3

all: Symulator.o Models.o test.o main.o
	g++ $(FLAGS) Models.o test.o Symulator.o -o test
	g++ $(FLAGS) Models.o main.o Symulator.o -o symulator

Models.o: Models.cpp Models.h
	g++ -c $(FLAGS) Models.cpp

Symulator.o: Symulator.h Symulator.cpp
	g++ -c $(FLAGS) Symulator.cpp

test.o: test.cpp
	g++ -c $(FLAGS) test.cpp

main.o: main.cpp
	g++ -c $(FLAGS) main.cpp

clean:
	rm *.o
	rm test symulator
