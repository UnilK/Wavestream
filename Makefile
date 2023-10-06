CXX := g++
RESLIB := bin/wstream.a
TEST := test

TESTDIR := test
SRCDIR := src
BUILDDIR := build
BINDIR := bin
LIBDIR := lib
HEADIR := include

SOURCES := $(shell find $(SRCDIR) -name *.cpp)
OBJECTS := $(patsubst $(SRCDIR)%, $(BUILDDIR)%, $(SOURCES:.cpp=.o))

SRCSTRUCT := $(shell find $(SRCDIR) -type d)
BUILDSTRUCT := $(patsubst $(SRCDIR)%, $(BUILDDIR)%, $(SRCSTRUCT))

INC := -I include

#CXXFLAGS := -std=c++17 -Og -g -Wall
CXXFLAGS := -std=c++17 -march=native -O3 -Wall -Wextra

$(RESLIB): $(BUILDSTRUCT) $(OBJECTS)
	@mkdir -p $(BINDIR)
	ar crs $(RESLIB) $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

$(BUILDSTRUCT):
	@mkdir -p $(BUILDSTRUCT)

.PHONY: test
test: $(RESLIB)
	$(CXX) -c $(CXXFLAG) $(INC) $(TESTDIR)/$(TEST).cpp -o $(BUILDDIR)/$(TEST).o
	$(CXX) $(BUILDDIR)/$(TEST).o $(RESLIB) -o $(BINDIR)/$(TEST)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(BINDIR)

.PHONY: depend
depend: $(SOURCES) $(TESTDIR)/$(TEST).cpp
	makedepend -Y$(HEADIR) $^
	@sed -i -e "s/$(SRCDIR)\//$(BUILDDIR)\//g" Makefile
	@sed -i -e "s/$(TESTDIR)\//$(BUILDDIR)\//g" Makefile

# DO NOT DELETE THIS LINE -- make depend depends on it. 

build/wstream/wave_dialog.o: include/wstream/wave_dialog.h
build/wstream/iwstream.o: include/wstream/wstream.h
build/wstream/iwstream.o: include/wstream/wave_dialog.h
build/wstream/iwstream.o: include/wstream/constants.h
build/wstream/waveconfig.o: include/wstream/wstream.h
build/wstream/waveconfig.o: include/wstream/wave_dialog.h
build/wstream/waveconfig.o: include/wstream/constants.h
build/wstream/owstream.o: include/wstream/wstream.h
build/wstream/owstream.o: include/wstream/wave_dialog.h
build/wstream/owstream.o: include/wstream/constants.h
build/test.o: include/wstream/wave_dialog.h include/wstream/wstream.h
