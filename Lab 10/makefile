all: server

server: server.o handle.o utility.o
	g++ -pthread -o server server.o handle.o utility.o

server.o: server10.cpp declarations.h 
	g++ -c -pthread server10.cpp -o server.o

handle.o: handlereq.cpp 
	g++ -c -pthread handlereq.cpp -o handle.o

utility.o: utility.cpp 
	g++ -c -pthread utility.cpp -o utility.o

clean: 
	rm -f *.o server
