CC=g++
CFLAGS=-Wall -Werror -pedantic-errors -std=c++0x -O0 -ggdb
TFLAGS=$(CFLAGS) -lboost_unit_test_framework

box.o: box.cpp box.hpp
	$(CC) -c box.cpp $(CFLAGS)

treenode.o: treenode.cpp treenode.hpp
	$(CC) -c treenode.cpp $(CFLAGS)

tests: tests.cpp box.o treenode.o
	$(CC) -o tests tests.cpp box.o treenode.o $(TFLAGS)

clean:
	rm -f box.o
	rm -f treenode.o
	rm -f tests
