all: test1 test2 sizetest sertest sltest

bdrwlock.o : bdrwlock.h bdrwlock.cpp 
	g++ -g -std=c++14 -Wall -c -o bdrwlock.o bdrwlock.cpp

dbstring.o : dbstring.cpp  dbstring.h sharedobj.h bdrwlock.h lookup3.h
	g++ -g -std=c++14 -Wall -c -o dbstring.o dbstring.cpp

lookup3.o : lookup3.c lookup3.h
	g++ -g -std=c++14 -Wall -c -o lookup3.o lookup3.c

test1.o : test1.cpp sharedobj.h dbstring.h
	g++ -g -std=c++14 -Wall -c -o test1.o test1.cpp

test1 : test1.o sharedobj.h dbstring.h dbstring.o bdrwlock.o lookup3.o
	g++ -g -std=c++14 -Wall -o test1 test1.o bdrwlock.o dbstring.o lookup3.o -lpthread 

test2.o : test2.cpp sharedobj.h dbstring.h dbstring.o
	g++ -g -std=c++14 -Wall -c -o test2.o test2.cpp

test2 : test2.o dbstring.o bdrwlock.o lookup3.o
	g++ -g -std=c++14 -Wall -o test2 test2.o bdrwlock.o dbstring.o lookup3.o -lpthread 

sizetest.o : sizetest.cpp sharedobj.h dbstring.h
	g++ -g -std=c++14 -Wall -c -o sizetest.o sizetest.cpp 

sizetest : sizetest.o dbstring.o bdrwlock.o lookup3.o
	g++ -g -std=c++14 -Wall -o sizetest sizetest.o bdrwlock.o dbstring.o lookup3.o -lpthread 

sertest.o : sertest.cpp sharedobj.h dbstring.h
	g++ -g -std=c++14 -Wall -c -o sertest.o sertest.cpp

sertest : sertest.o dbstring.o bdrwlock.o lookup3.o
	g++ -g -std=c++14 -Wall -o sertest sertest.o bdrwlock.o dbstring.o lookup3.o -lpthread -lboost_serialization 

sltest.o : sltest.cpp sharedobj.h dbstring.h
	g++ -g -std=c++14 -Wall -c -o sltest.o sltest.cpp

sltest : sltest.o dbstring.o bdrwlock.o lookup3.o
	g++ -g -std=c++14 -Wall -o sltest sltest.o bdrwlock.o dbstring.o lookup3.o -lpthread -lboost_serialization 

.Phony: clean

clean:
	-rm -f *.o
	-rm -f test1
	-rm -f test2
	-rm -f sizetest
	-rm -f sertest
	-rm -f sltest
