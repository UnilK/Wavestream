#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>

#include "wave_dialog.h"
#include "wavestream.h"

using std::cin;
using std::cout;

int main(){
	
	srand(time(0));

	cout << "wave_dialog tests only work on little endian machines.\n"
		 <<"This doesn't neccessarily mean that the functions do not work.\n";

	bool ok = 1;
	for(int i=0; i<10000; i++){
		uint16_t t = rand()&((1<<16)-1);
		uint16_t rr = wave_dialog::listen_uint16((char*)&t);
		if(t != rr) ok = 0;
	}

	cout << "wave_dialog::listen_uint16: " << ok << '\n';
	
	ok = 1;
	for(int i=0; i<10000; i++){
		uint32_t t = ((long long)123*rand()+rand())&((1ll<<32)-1);
		uint32_t rr = wave_dialog::listen_uint32((char*)&t);
		if(t != rr) ok = 0;
	}

	cout << "wave_dialog::listen_uint32: " << ok << '\n';
	cout << '\n';


	
	ok = 1;
	for(int i=0; i<1000; i++){
		int16_t t = rand()&((1<<8)-1);
		float tt = (float)(t-128)/(1<<7);
		float rr = wave_dialog::listen_int8_as_float((char*)&t);
		if(tt != rr) ok = 0;
	}
	
	cout << "wave_dialog::listen_int8_as_float: " << ok << '\n';

	ok = 1;
	for(int i=0; i<10000; i++){
		int16_t t = rand()&((1<<16)-1);
		float tt = (float)t/(1<<15);
		float rr = wave_dialog::listen_int16_as_float((char*)&t);
		if(tt != rr) ok = 0;
	}

	cout << "wave_dialog::listen_int16_as_float: " << ok << '\n';
	
	ok = 1;
	for(int i=0; i<10000; i++){
		int32_t t = rand()&((1<<24)-1);
		int32_t ttt = (t<<8)/(1<<8);
		float tt = (float)ttt/(1<<23);
		float rr = wave_dialog::listen_int24_as_float((char*)&t);
		if(tt != rr) ok = 0;
	}

	cout << "wave_dialog::listen_int24_as_float: " << ok << '\n';
	
	ok = 1;
	for(int i=0; i<10000; i++){
		int32_t t = ((long long)123*rand()+rand())&((1ll<<32)-1);
		if(rand()&1) t = -t;
		float tt = (float)t/(1ll<<31);
		float rr = wave_dialog::listen_int32_as_float((char*)&t);
		if(tt != rr) ok = 0;
	}
	
	cout << "wave_dialog::listen_int32_as_float: " << ok << '\n';

	ok = 1;
	for(int i=0; i<10000; i++){
		float t = (float)rand()/(1<<30);
		if(rand()&1) t = -t;
		float rr = wave_dialog::listen_float32((char*)&t);
		if(t != rr) ok = 0;
	}
	
	cout << "wave_dialog::listen_float32: " << ok << '\n';
	
	cout << '\n';


	
	ok = 1;
	for(int i=0; i<10000; i++){
		uint16_t t = rand()&((1<<16)-1);
		uint16_t rr = wave_dialog::listen_uint16((char*)&t), pp = 0;
		wave_dialog::say_uint16(rr, (char*)&pp);
		if(rr != pp) ok = 0;
	}
	
	cout << "wave_dialog::say_uint16: " << ok << '\n';
	
	ok = 1;
	for(int i=0; i<10000; i++){
		uint32_t t = ((long long)123*rand()+rand())&((1ll<<32)-1);
		uint32_t rr = wave_dialog::listen_uint32((char*)&t), pp = 0;
		wave_dialog::say_uint32(rr, (char*)&pp);
		if(rr != pp) ok = 0;
	}
	
	cout << "wave_dialog::say_uint32: " << ok << '\n';
	cout << '\n';



	ok = 1;
	for(int i=0; i<10000; i++){
		int8_t t = rand()&((1<<8)-1);
		float rr = wave_dialog::listen_int8_as_float((char*)&t);
		
		int8_t nn;
		wave_dialog::say_float_as_int8(rr, (char*)&nn);

		if(nn != t) ok = 0;

	}
	cout << "wave_dialog::say_float_as_int8: " << ok << '\n';

	ok = 1;
	for(int i=0; i<10000; i++){
		int16_t t = rand()&((1<<16)-1);
		float rr = wave_dialog::listen_int16_as_float((char*)&t);

		int16_t nn;
		wave_dialog::say_float_as_int16(rr, (char*)&nn);

		if(nn != t) ok = 0;
	}
	
	cout << "wave_dialog::say_float_as_int16: " << ok << '\n';
	
	ok = 1;
	for(int i=0; i<10000; i++){
		int32_t t = rand()&((1<<24)-1);
		int32_t ttt = (t<<8)/(1<<8);
		float rr = wave_dialog::listen_int24_as_float((char*)&t);

		int32_t nn;
		wave_dialog::say_float_as_int24(rr, (char*)&nn);

		char *cc = (char*)&nn;
		if((cc[2]>>7)&1){
			cc[3] = 0xff;
		} else {
			cc[3] = 0x00;
		}

		if(nn != ttt) ok = 0;
	}
	
	cout << "wave_dialog::say_float_as_int24: " << ok << '\n';

	ok = 1;
	for(int i=0; i<10000; i++){
		int32_t t = ((long long)123*rand()+rand())&((1ll<<32)-1);
		float rr = wave_dialog::listen_int32_as_float((char*)&t);

		int32_t nn;
		wave_dialog::say_float_as_int32(rr, (char*)&nn);
		if(std::abs((nn/(1<<8))-(t/(1<<8))) > 1) ok = 0;
	}
	
	cout << "wave_dialog::say_float_as_int32: " << ok << '\n';
	
	ok = 1;
	for(int i=0; i<10000; i++){
		float t = (float)rand()/(1<<30);
		if(rand()&1) t = -t;
		float rr = wave_dialog::listen_float32((char*)&t), pp = 0;
		wave_dialog::say_float32(rr, (char*)&pp);
		if(t != pp) ok = 0;
	}
	
	cout << "wave_dialog::say_float32: " << ok << '\n';

	// these tests should work on all machines

	cout << "mono files at 44,1 kHz for rigorous testing,\n"
		<< "more than one channel or other frequencies for funny audio.\n"
		<< "input test file name:\n";

	std::string ifile;
	cin >> ifile;

	iwavestream I(ifile);

	std::vector<float> w;
    float *w2 = new float[I.get_frame_amount()*I.get_channel_amount()];

	I.read_file(&w);
	I.read_file(w2);

	
	cout << "input file:\n";
	for(auto i : I.get_log()) cout << i << '\n';
	cout << '\n';

	// owavestream O0("t0.wav", 0x1, 1, 8, 44100); need to find some media player that supports this
	owavestream O1("t1.wav", 0x1, 1, 16, 44100);
	owavestream O2("t2.wav", 0x1, 1, 24, 44100);
	owavestream O3("t3.wav", 0x1, 1, 32, 44100);
	owavestream O4("t4.wav", 0x3, 1, 32, 44100);
	
	owavestream O5("t5.wav", 0xfffe, 1, 16, 44100, 0x1, 1);
	owavestream O6("t6.wav", 0xfffe, 1, 32, 44100, 0x3, 1);
	
    owavestream O7("t7.wav", &I);

	// O0.write_file(&w);
	O1.write_file(&w);
	O2.write_file(&w);
	O3.write_file(&w);
	O4.write_file(w2, I.get_frame_amount());
	O5.write_file(w2, I.get_frame_amount());
	O6.write_file(w2, I.get_frame_amount());

    for(int i=0; i<3; i++) O7.write_samples(&w);
    O7.close();


    I.close();

	std::vector<float> w3;
    iwavestream I2(ifile);

    for(int i=0; i<5; i++){
        I2.read_frames(&w3, 1000, I2.get_frame_amount()/2);
        while(I2.read_frames(&w3, 1000));
    }

    owavestream O8;
    O8.config(0x1, 1, 16, 44100);
    O8.open("t8.wav");
    O8.initialize();
    O8.write_file(&w3);
	
	// for(auto i : O0.get_log()) cout << i << '\n';
	// cout << '\n';

	cout << "t1, PCM, 1 channel, 16 bits at 44,1 kHz:\n";
	for(auto i : O1.get_log()) cout << i << '\n';
	cout << '\n';
	cout << "t2, PCM, 1 channel, 24 bits at 44,1 kHz:\n";
	for(auto i : O2.get_log()) cout << i << '\n';
	cout << '\n';
	cout << "t3, PCM, 1 channel, 32 bits at 44,1 kHz:\n";
	for(auto i : O3.get_log()) cout << i << '\n';
	cout << '\n';
	cout << "t4, IEEE, 1 channel, 32 bits at 44,1 kHz:\n";
	for(auto i : O4.get_log()) cout << i << '\n';
	cout << '\n';
	cout << "t5, WAVE_FORMAT_EXTENSIBLE with subformat PCM, 1 channel, 16 bits at 44,1 kHz:\n";
	for(auto i : O5.get_log()) cout << i << '\n';
	cout << '\n';
	cout << "t6, WAVE_FORMAT_EXTENSIBLE with subformat IEEE, 1 channel, 32 bits at 44,1 kHz:\n";
	for(auto i : O6.get_log()) cout << i << '\n';
	cout << '\n';
    cout << "t7, initialized from test file & written in bits:";
    for(auto i : O7.get_config()) cout << i << ' ';
    cout << '\n';
	for(auto i : O7.get_log()) cout << i << '\n';
	cout << '\n';
	cout << "t8, PCM, 1 channel, 16 bits at 44,1 kHz, file is read in a silly way:\n";
	for(auto i : I2.get_log()) cout << i << '\n';
	cout << '\n';
	for(auto i : O8.get_log()) cout << i << '\n';
	cout << '\n';

	return 0;
}

