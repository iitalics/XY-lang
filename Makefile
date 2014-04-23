CXX=g++
CXXFLAGS=-Wall -Wextra -std=c++11

LINK=g++
LINKFLAGS=-lm -O3


OUTPUT=xy
SOURCES=		main.cpp


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