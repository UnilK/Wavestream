CXX := g++
RESLIB := bin/wavestream.a
TEST := test

TESTDIR := test
SRCDIR := src
BUILDDIR := build
BINDIR := bin
LIBDIR := lib

SOURCES := $(shell find $(SRCDIR) -name *.cpp)
OBJECTS := $(patsubst $(SRCDIR)%, $(BUILDDIR)%, $(SOURCES:.cpp=.o))

SRCSTRUCT := $(shell find $(SRCDIR) -type d)
BUILDSTRUCT := $(patsubst $(SRCDIR)%, $(BUILDDIR)%, $(SRCSTRUCT))

INC := -I include

#CXXFLAGS := -std=c++17 -g -Wall -fsanitize=leak
CXXFLAGS := -std=c++17 -O2 -Wall

$(RESLIB): $(BUILDSTRUCT) $(OBJECTS)
	@echo "mkdir -p $(BINDIR)"
	@mkdir -p $(BINDIR)
	ar crs $(RESLIB) $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

$(BUILDSTRUCT):
	@echo "mkdir -p $(BUILDSTRUCT)"
	@mkdir -p $(BUILDSTRUCT)

.PHONY: test
test: $(RESLIB)
	$(CXX) -c $(CXXFLAG) $(INC) $(TESTDIR)/$(TEST).cpp -o $(BUILDDIR)/$(TEST).o
	$(CXX) $(BUILDDIR)/$(TEST).o $(RESLIB) -o $(BINDIR)/$(TEST)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(BINDIR)

