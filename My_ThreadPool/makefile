hello: ThreadPool.o main.o
	g++ main.o ThreadPool.o -o hello

ThreadPool.o: ThreadPool.cpp
	g++ ThreadPool.cpp -c ThreadPool.o

main.o: main.cpp
	g++ main.cpp -c main.o

clean:
	rm -f *.o hello
