CXX=g++
CXXFLAGS=-Wall -Wextra -Wno-unused-parameter -std=c++11

LINK=g++
LINKFLAGS=-lm -O3


OUTPUT=xy
SOURCES=		main.cpp state.cpp lexer.cpp error.cpp \
				environment.cpp parser.cpp value.cpp


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