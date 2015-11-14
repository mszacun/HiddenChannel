FLAGS=-std=c++11 -ggdb

all: Symulator.o Models.o test.o
	g++ $(FLAGS) Models.o test.o Symulator.o -o test

Models.o: Models.cpp Models.h
	g++ -c $(FLAGS) Models.cpp

Symulator.o: Symulator.h Symulator.cpp
	g++ -c $(FLAGS) Symulator.cpp

test.o: test.cpp
	g++ -c $(FLAGS) test.cpp

clean:
	rm *.o
	rm test
