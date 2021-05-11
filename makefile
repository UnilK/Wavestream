
CXX := g++
TEST := test

SOURCES := $(shell find src -name *.cpp)
OBJECTS := $(patsubst src/%, build/%, $(SOURCES:.cpp=.o))

INC := -I include
#CXXFLAGS := -std=c++17 -g -Wall -fsanitize=leak
CXXFLAGS := -std=c++17 -O3 -Wall

$(TEST): $(OBJECTS) build/$(TEST).o
	$(CXX) $^ -o bin/$(TEST)

build/$(TEST).o: test/$(TEST).cpp 
	@mkdir -p build
	$(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

build/%.o: src/%.cpp 
	@mkdir -p build
	$(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	rm -rf build
	rm -rf $(TARGET)

