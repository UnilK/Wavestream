#include "wave_dialog.h"

namespace wave_dialog{

    uint16_t listen_uint16(const char *r){
        const uint8_t *c = (uint8_t*)r;
        return (uint16_t)c[0] | c[1]<<8;
    }

    uint32_t listen_uint32(const char *r){
        const uint8_t *c = (uint8_t*)r;
        return (uint32_t)c[0] | c[1]<<8 | c[2]<<16 | c[3]<<24;
    }



    float listen_int8_as_float(const char *r){
        const uint8_t *c = (uint8_t*)r;
        int16_t x = (int16_t)c[0] - 128;
        return (float)x/(1<<7);
    }

    float listen_int16_as_float(const char *r){
        const uint8_t *c = (uint8_t*)r;
        int16_t x = (int16_t)c[0] | c[1]<<8;
        return (float)x/(1<<15);
    }
    
    float listen_int24_as_float(const char *r){
        const uint8_t *c = (uint8_t*)r;
        int32_t x = ((int32_t)c[0]<<8 | c[1]<<16 | c[2]<<24)/(1<<8);
        return (float)x/(1<<23);
    }
    
    float listen_int32_as_float(const char *r){
        const uint8_t *c = (uint8_t*)r;
        int32_t x = (int32_t)c[0] | c[1]<<8 | c[2]<<16 | c[3]<<24;
        return (float)x/(1ll<<31);
    }
 
    float listen_float32(const char *r){
        const uint8_t *c = (uint8_t*)r;
        const int32_t x = (int32_t)c[0] | c[1]<<8 | c[2]<<16 | c[3]<<24;
        const char *t = (char*)&x;
        return *((float*)t);
    }



    void say_uint16(const uint16_t x, char *c){
        
        const uint8_t *cx = (uint8_t*)&x;
        const uint16_t b = (uint16_t)cx[0] | cx[1]<<8;
        const char *cb = (char*)&b;

        c[0] = cb[0];
        c[1] = cb[1];
    }
    
    void say_uint32(const uint32_t x, char *c){
        
        const uint8_t *cx = (uint8_t*)&x;
        const uint32_t b = (uint32_t)cx[0] | cx[1]<<8 | cx[2]<<16 | cx[3]<<24;
        const char *cb = (char*)&b;

        c[0] = cb[0];
        c[1] = cb[1];
        c[2] = cb[2];
        c[3] = cb[3];
    }

    

    void say_float_as_int8(const float x, char *c){
        const uint8_t t = (uint8_t)std::min(255.0f, std::max(0.0f, (x+1)*128));
        c[0] = t;
    }

    void say_float_as_int16(const float x, char *c){
        const int16_t t = (int16_t)std::min((float)((1<<15)-1),
                std::max(-(float)(1<<15), x*(1<<15)));
        c[0] = t&0x00ff;    
        c[1] = (t&0xff00)>>8;   
    }
    
    void say_float_as_int24(const float x, char *c){
        const int32_t t = (int32_t)std::min((float)((1<<23)-1),
                std::max(-(float)(1<<23), x*(1<<23)));
        c[0] = t&0x0000ff;
        c[1] = (t&0x00ff00)>>8;
        c[2] = (t&0xff0000)>>16 | ((t>>31)&1)<<7;
    }


    void say_float_as_int32(const float x, char *c){
        const int32_t t = (int32_t)std::min((float)((1ll<<31)-1),
                std::max(-(float)(1ll<<31), x*(1ll<<31)));
        c[0] = t&0x000000ff;
        c[1] = (t&0x0000ff00)>>8;
        c[2] = (t&0x00ff0000)>>16;
        c[3] = (t&0xff000000)>>24;
    }

    void say_float32(const float x, char *c){
        
        const uint8_t *cx = (uint8_t*)&x;
        const uint32_t b = (uint32_t)cx[0] | cx[1]<<8 | cx[2]<<16 | cx[3]<<24;
        const char *cb = (char*)&b;

        c[0] = cb[0];
        c[1] = cb[1];
        c[2] = cb[2];
        c[3] = cb[3];
    }

    uint32_t resolve_dialog(uint16_t format, uint16_t bits){
        uint32_t id = (uint32_t)format<<16 | bits;
        switch(id){
            case INT8_ID:
                return id;
            case INT16_ID:
                return id;
            case INT24_ID:
                return id;
            case INT32_ID:
                return id;
            case FLOAT32_ID:
                return id;
        }
        return 0;
    }

}

