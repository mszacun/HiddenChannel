FLAGS=-std=c++11 -ggdb

all: Symulator.o Models.o test.o main.o main2.o
	g++ $(FLAGS) Models.o test.o Symulator.o -o test
	g++ $(FLAGS) Models.o main.o Symulator.o -o symulator
	g++ $(FLAGS) Models.o main2.o Symulator.o -o single_symulator

Models.o: Models.cpp Models.h
	g++ -c $(FLAGS) Models.cpp

Symulator.o: Symulator.h Symulator.cpp
	g++ -c $(FLAGS) Symulator.cpp

test.o: test.cpp
	g++ -c $(FLAGS) test.cpp

main.o: main.cpp
	g++ -c $(FLAGS) main.cpp

main2.o: main2.cpp
	g++ -c $(FLAGS) main2.cpp

clean:
	rm *.o
	rm test symulator
