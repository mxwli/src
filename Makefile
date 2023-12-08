EXEC=./vm
CXX=g++
CXXFLAGS=-std=c++20 -MMD -g
LIB=-lncurses
SRC=$(wildcard *.cc)
OBJECTS=$(SRC:.cc=.o)
DEPENDS=$(OBJECTS:.o=.d)

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC} ${LIB}

-include ${DEPENDS}

.PHONY: clean run

clean:
	rm ${OBJECTS} ${DEPENDS} ${EXEC}

run: ${EXEC}
	@echo ~~~~~~~~~~~~~
	@${EXEC}
