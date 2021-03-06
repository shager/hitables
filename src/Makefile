CC=g++
CFLAGS=-Wall -Werror -pedantic-errors -std=c++0x -O3
TFLAGS=$(CFLAGS) -lboost_unit_test_framework

hitables: hitables_main.cpp box.o rule.o action.o parse.o treenode.o arg.o \
emit.o
	$(CC) -o hitables hitables_main.cpp box.o rule.o action.o parse.o \
	treenode.o arg.o emit.o $(CFLAGS)

tests: tests.cpp box.o rule.o action.o parse.o treenode.o arg.o emit.o
	$(CC) -o tests tests.cpp box.o rule.o action.o parse.o treenode.o arg.o \
	emit.o $(TFLAGS)

remove_redundancy: remove_redundancy.cpp parse.o
	$(CC) -o remove_redundancy remove_redundancy.cpp parse.o $(CFLAGS)

box.o: box.cpp box.hpp
	$(CC) -c box.cpp $(CFLAGS)

rule.o: rule.cpp rule.hpp
	$(CC) -c rule.cpp $(CFLAGS)

action.o: action.cpp action.hpp
	$(CC) -c action.cpp $(CFLAGS)

parse.o: parse.cpp parse.hpp
	$(CC) -c parse.cpp $(CFLAGS)

treenode.o: treenode.cpp treenode.hpp
	$(CC) -c treenode.cpp $(CFLAGS)

arg.o: arg.cpp arg.hpp
	$(CC) -c arg.cpp $(CFLAGS)

emit.o: emit.cpp emit.hpp
	$(CC) -c emit.cpp $(CFLAGS)

clean:
	rm -f box.o
	rm -f rule.o
	rm -f action.o
	rm -f parse.o
	rm -f treenode.o
	rm -f arg.o
	rm -f emit.o
	rm -f tests
	rm -f hitables
	rm -f remove_redundancy
