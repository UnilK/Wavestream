#ifndef WAVE_DIALOG_H
#define WAVE_DIALOG_H

#include <cstdint>
#include <algorithm>

namespace wave_dialog{

    const uint32_t PCM_ID =     0x0001;
    const uint32_t IEEE_ID =    0x0003;

    const uint32_t INT8_ID =    0x0008 | PCM_ID<<16;
    const uint32_t INT16_ID =   0x0010 | PCM_ID<<16;
    const uint32_t INT24_ID =   0x0018 | PCM_ID<<16;
    const uint32_t INT32_ID =   0x0020 | PCM_ID<<16;
    const uint32_t FLOAT32_ID = 0x0020 | IEEE_ID<<16;
	
    uint16_t listen_uint16(const char *buffer);
	uint32_t listen_uint32(const char *buffer);

	float listen_int8_as_float(const char *buffer);
	float listen_int16_as_float(const char *buffer);
	float listen_int24_as_float(const char *buffer);
	float listen_int32_as_float(const char *buffer);
	float listen_float32(const char *buffer);

	void say_uint16(const uint16_t number, char *buffer);
	void say_uint32(const uint32_t number, char *buffer);

	void say_float_as_int8(const float number, char *buffer);
	void say_float_as_int16(const float number, char *buffer);
	void say_float_as_int24(const float number, char *buffer);
	void say_float_as_int32(const float number, char *buffer);
	void say_float32(const float number, char *buffer);

	uint32_t resolve_dialog(uint16_t format, uint16_t bits);
}

#endif
