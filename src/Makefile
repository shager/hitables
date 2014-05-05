CC=g++
CFLAGS=-Wall -Werror -pedantic-errors -std=c++0x -O0
TFLAGS=$(CFLAGS) -lboost_unit_test_framework

hitables: hitables_main.cpp box.o action.o parse.o treenode.o
	$(CC) -o hitables hitables_main.cpp box.o action.o parse.o treenode.o arg.o $(CFLAGS)

tests: tests.cpp box.o action.o parse.o treenode.o arg.o
	$(CC) -o tests tests.cpp box.o action.o parse.o treenode.o arg.o $(TFLAGS)

box.o: box.cpp box.hpp
	$(CC) -c box.cpp $(CFLAGS)

action.o: action.cpp action.hpp
	$(CC) -c action.cpp $(CFLAGS)

parse.o: parse.cpp parse.hpp rule.hpp partial_ruleset.hpp
	$(CC) -c parse.cpp $(CFLAGS)

treenode.o: treenode.cpp treenode.hpp
	$(CC) -c treenode.cpp $(CFLAGS)

arg.o: arg.cpp arg.hpp
	$(CC) -c arg.cpp $(CFLAGS)

clean:
	rm -f box.o
	rm -f action.o
	rm -f parse.o
	rm -f treenode.o
	rm -f arg.o
	rm -f tests
	rm -f hitables
