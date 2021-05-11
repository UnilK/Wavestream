
Provides classes iwavestream and owavestream for
reading & writing .wav files. The interface provided is:
wave file -> vector\<float\>
vector\<float\> -> wave file

The supported formats are:

0x0001	WAVE\_FORMAT\_PCM
8, 16, 24 and 32 bits
32 bit PCM will suffer 8 bits of quality loss.

0x0003	WAVE\_FORMAT\_IEEE\_FLOAT
32 bits

0xfffe	WAVE\_FORMAT\_EXTENSIBLE
with subformats 0x0001 and 0x0003.


Compiling tests:

Linux/(iOS):

Run the makefile with command "make" to compile the tests.
It produces the following commands:

g++ -c -std=c++17 -O3 -Wall -I include src/owavestream.cpp -o build/owavestream.o
g++ -c -std=c++17 -O3 -Wall -I include src/wave\_dialog.cpp -o build/wave\_dialog.o
g++ -c -std=c++17 -O3 -Wall -I include src/waveconfig.cpp -o build/waveconfig.o
g++ -c -std=c++17 -O3 -Wall -I include src/iwavestream.cpp -o build/iwavestream.o
g++ -c -std=c++17 -O3 -Wall -I include test/test.cpp -o build/test.o
g++ build/owavestream.o build/wave\_dialog.o build/waveconfig.o build/iwavestream.o build/test.o -o bin/test

(Windows):

Run the commands produced by the makefile.

