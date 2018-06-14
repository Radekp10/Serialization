#makefile

__start__: a.out
	./a.out

a.out: serialization.o 
	g++ serialization.o -lboost_serialization

serialization.o: serialization.cpp
	g++ -c -Wall serialization.cpp 

clean: 
	rm serialization.o a.out
