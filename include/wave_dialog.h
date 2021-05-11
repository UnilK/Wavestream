#ifndef WAVE_DIALOG_H
#define WAVE_DIALOG_H

#include <cstdint>
#include <algorithm>

namespace wave_dialog{

	uint16_t listen_uint16(char *buffer);
	uint32_t listen_uint32(char *buffer);

	float listen_int8_as_float(char *buffer);
	float listen_int16_as_float(char *buffer);
	float listen_int24_as_float(char *buffer);
	float listen_int32_as_float(char *buffer);
	float listen_float32(char *buffer);

	void say_uint16(uint16_t number, char *buffer);
	void say_uint32(uint32_t number, char *buffer);

	void say_float_as_int8(float number, char *buffer);
	void say_float_as_int16(float number, char *buffer);
	void say_float_as_int24(float number, char *buffer);
	void say_float_as_int32(float number, char *buffer);
	void say_float32(float number, char *buffer);

	bool valid_dialog(uint16_t format, uint16_t bytes);

	typedef float (*listener)(char*);
	listener resolve_listener(uint16_t format, uint16_t bits);

	typedef void (*speaker)(float, char*);
	speaker resolve_speaker(uint16_t format, uint16_t bits);

	std::pair<listener, speaker> resolve_dialog(uint16_t format, uint16_t bits);
}

#endif
