#include "wave_dialog.h"

namespace wave_dialog{

	uint16_t listen_uint16(char *r){
		uint8_t *c = (uint8_t*)r;
		return (uint16_t)c[0] | c[1]<<8;
	}

	uint32_t listen_uint32(char *r){
		uint8_t *c = (uint8_t*)r;
		return (uint32_t)c[0] | c[1]<<8 | c[2]<<16 | c[3]<<24;
	}



	float listen_int8_as_float(char *r){
		uint8_t *c = (uint8_t*)r;
		int16_t x = (int16_t)c[0] - 128;
		return (float)x/(1<<7);
	}

	float listen_int16_as_float(char *r){
		uint8_t *c = (uint8_t*)r;
		int16_t x = (int16_t)c[0] | c[1]<<8;
		return (float)x/(1<<15);
	}
	
	float listen_int24_as_float(char *r){
		uint8_t *c = (uint8_t*)r;
		int32_t x = ((int32_t)c[0]<<8 | c[1]<<16 | c[2]<<24)/(1<<8);
		return (float)x/(1<<23);
	}
	
	float listen_int32_as_float(char *r){
		uint8_t *c = (uint8_t*)r;
		int32_t x = (int32_t)c[0] | c[1]<<8 | c[2]<<16 | c[3]<<24;
		return (float)x/(1ll<<31);
	}
 
	float listen_float32(char *r){
		
		/*
		   support for both little and big endian systems results
		   in some pretty ugly code... I hope this at least compiles
		   to something neat with the right flags. This doesn't work
		   if floating points and integers have different endianness.
		*/

		uint8_t *c = (uint8_t*)r;
		uint32_t x = (uint32_t)c[0] | c[1]<<8 | c[2]<<16 | c[3]<<24;

		float xx = 0;

		uint8_t *c1 = (uint8_t*)&x;
		uint8_t *c2 = (uint8_t*)&xx;

		c2[0] = c1[0];
		c2[1] = c1[1];
		c2[2] = c1[2];
		c2[3] = c1[3];

		return xx;
	}



	void say_uint16(uint16_t x, char *c){
		
		uint8_t *cx = (uint8_t*)&x;
		uint16_t b = (uint16_t)cx[0] | cx[1]<<8;
		char *cb = (char*)&b;

		c[0] = cb[0];
		c[1] = cb[1];
	}
	
	void say_uint32(uint32_t x, char *c){
		
		uint8_t *cx = (uint8_t*)&x;
		uint32_t b = (uint32_t)cx[0] | cx[1]<<8 | cx[2]<<16 | cx[3]<<24;
		char *cb = (char*)&b;

		c[0] = cb[0];
		c[1] = cb[1];
		c[2] = cb[2];
		c[3] = cb[3];
	}

	

	void say_float_as_int8(float x, char *c){
		uint8_t t = (uint8_t)std::min(255.0f, (x+1)*128);
		c[0] = t;
	}

	void say_float_as_int16(float x, char *c){
		int16_t t = (int16_t)std::min((float)((1<<15)-1),
				std::max(-(float)(1<<15), x*(1<<15)));
		c[0] = t&0x00ff;	
		c[1] = (t&0xff00)>>8;	
	}
	
	void say_float_as_int24(float x, char *c){
		int32_t t = (int32_t)std::min((float)((1<<23)-1),
				std::max(-(float)(1<<23), x*(1<<23)));
		c[0] = t&0x0000ff;
		c[1] = (t&0x00ff00)>>8;
		c[2] = (t&0xff0000)>>16 | ((t>>31)&1)<<7;
	}


	void say_float_as_int32(float x, char *c){
		int32_t t = (int32_t)std::min((float)((1ll<<31)-1),
				std::max(-(float)(1ll<<31), x*(1ll<<31)));
		c[0] = t&0x000000ff;
		c[1] = (t&0x0000ff00)>>8;
		c[2] = (t&0x00ff0000)>>16;
		c[3] = (t&0xff000000)>>24;
	}

	void say_float32(float x, char *c){
		
		uint8_t *cx = (uint8_t*)&x;
		uint32_t b = (uint32_t)cx[0] | cx[1]<<8 | cx[2]<<16 | cx[3]<<24;
		char *cb = (char*)&b;

		c[0] = cb[0];
		c[1] = cb[1];
		c[2] = cb[2];
		c[3] = cb[3];
	}



	const uint32_t PCM_ID = 	0x0001;
	const uint32_t IEEE_ID = 	0x0003;

	const uint32_t INT8_ID =	0x0008 | PCM_ID<<16;
	const uint32_t INT16_ID = 	0x0010 | PCM_ID<<16;
	const uint32_t INT24_ID =	0x0018 | PCM_ID<<16;
	const uint32_t INT32_ID = 	0x0020 | PCM_ID<<16;
	const uint32_t FLOAT32_ID = 0x0020 | IEEE_ID<<16;

	bool valid_dialog(uint16_t format, uint16_t bits){

		uint32_t id = (uint32_t)format<<16 | bits;

		switch(id){
			case INT8_ID:
				return 1;
			case INT16_ID:
				return 1;
			case INT24_ID:
				return 1;
			case INT32_ID:
				return 1;
			case FLOAT32_ID:
				return 1;
		}

		return 0;
	}

	std::pair<listener, speaker> resolve_dialog(uint16_t format, uint16_t bits){

		uint32_t id = (uint32_t)format<<16 | bits;

		switch(id){
			case INT8_ID:
				return {listen_int8_as_float, say_float_as_int8};
			case INT16_ID:
				return {listen_int16_as_float, say_float_as_int16};
			case INT24_ID:
				return {listen_int24_as_float, say_float_as_int24};
			case INT32_ID:
				return {listen_int32_as_float, say_float_as_int32};
			case FLOAT32_ID:
				return {listen_float32, say_float32};
		}

		return {NULL, NULL};

	}

	listener resolve_listener(uint16_t format, uint16_t bits){
		return resolve_dialog(format, bits).first;
	}

	speaker resolve_speaker(uint16_t format, uint16_t bits){
		return resolve_dialog(format, bits).second;
	}

}

