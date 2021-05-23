
Wavestream provides classes iwavestream and owavestream for
reading & writing .wav files. The interface provided is:

wave file -> std::vector<float> 
std::vector<float> -> wave file



The supported formats are:

0x0001	WAVE_FORMAT_PCM
8, 16, 24 and 32 bits
32 bit PCM will suffer 8 bits of quality loss.

0x0003	WAVE_FORMAT_IEEE_FLOAT
32 bits

0xfffe	WAVE_FORMAT_EXTENSIBLE
with subformats 0x0001 and 0x0003.



Compiling with makefile & GCC on Linux:

Running the makefile with command "make" should
produce the following commands:

mkdir -p  build
g++ -c -std=c++17 -O3 -Wall -I include src/owavestream.cpp -o build/owavestream.o
g++ -c -std=c++17 -O3 -Wall -I include src/wave_dialog.cpp -o build/wave_dialog.o
g++ -c -std=c++17 -O3 -Wall -I include src/waveconfig.cpp -o build/waveconfig.o
g++ -c -std=c++17 -O3 -Wall -I include src/iwavestream.cpp -o build/iwavestream.o
mkdir -p bin
ar crs bin/wavestream.a  build/owavestream.o  build/wave_dialog.o  build/waveconfig.o  build/iwavestream.o

With wavestream.a compiled, running "make test"
compiles the tests:

g++ -c  -I include test/test.cpp -o build/test.o
g++ build/test.o bin/wavestream.a -o bin/test

