CXX=g++
CXXFLAGS=-Wall -Wextra -Wno-unused-parameter -std=c++11 -O3

LINK=g++
LINKFLAGS=-lm -O3


OUTPUT=xy
SOURCES=		main.cpp state.cpp lexer.cpp map.cpp \
				error.cpp list.cpp environment.cpp   \
				parser.cpp value.cpp function.cpp    \
				expression.cpp native_functions.cpp  \


OBJECTS=$(SOURCES:%.cpp=obj/%.o)





obj/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OUTPUT)
clean:
	rm -rf $(OUTPUT) $(OBJECTS)
rebuild: clean all

obj:
	mkdir obj


$(OUTPUT): obj $(OBJECTS)
	$(LINK) $(OBJECTS) $(LINKFLAGS) -o $(OUTPUT)