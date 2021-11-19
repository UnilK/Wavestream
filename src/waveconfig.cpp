#include "wavestream.h"
#include "wave_dialog.h"
#include "constants.h"

waveconfig::waveconfig(){}

waveconfig::waveconfig(uint16_t format, uint16_t channel_amount, uint16_t sample_size,
        uint32_t frame_rate, uint16_t subformat, uint32_t mask){    
    config(format, channel_amount, sample_size, frame_rate, subformat, mask);
}

waveconfig::waveconfig(waveconfig *other){
    copy_config(other);
}

void waveconfig::add_log(std::string message){
    log.push_back(message);
    if(log.size() >= 200){
        for(int i=0; i<100; i++) log[i] = log[i+100];
        log.resize(100);
    }
}

std::vector<std::string> waveconfig::get_log(){
    return log;
}



bool waveconfig::set_format(uint16_t format_){
    switch(format_){
        case PCM:
            formatSize = 16;
            extensionSize = 0;
            format = format_;
            return 1;
        case IEEE:
            formatSize = 18;
            extensionSize = 0;
            format = format_;
            return 1;
        case EXTENSIBLE:
            formatSize = 40;
            extensionSize = 22;
            format = format_;
            return 1;
    }
    
    if(logging){
        add_log("format "+std::to_string(format_)
            +" not recognized. Known formats are 0x0001, 0x0003 and 0xfffe");
    }
    return 0;
}

bool waveconfig::set_channel_amount(uint16_t channels_){
    
    if(channels_ > 18){
        if(logging) add_log(std::to_string(channels_)+" is too many channels. Max 18.");
        return 0;
    }

    channels = channels_;
    frameSize = channels*sampleSize;
    byteRate = frameRate*frameSize;
    
    return 1;
}

bool waveconfig::set_sample_bitsize(uint16_t sampleBits_){
    
    if(sampleBits_%8){
        add_log("sample "+std::to_string(sampleBits_)
                +" size not supported, must be a multiple of 8");
        return 0;
    }

    sampleBits = sampleBits_;
    sampleSize = sampleBits/8;
    frameSize = channels*sampleSize;
    byteRate = frameRate*frameSize;

    return 1;
}

bool waveconfig::set_frame_rate(uint32_t frameRate_){
    
    frameRate = frameRate_;
    byteRate = frameRate*frameSize;

    return 1;
}

bool waveconfig::set_subformat(uint16_t subformat_){

    if(format == EXTENSIBLE){
        
        switch(subformat_){
            case PCM:
                subformat = subformat_;
                return 1;
            case IEEE:
                subformat = subformat_;
                return 1;
        }
    
        if(logging){
            add_log("Subormat "+std::to_string(subformat_)
                +" not recognized. Supported formats are 0x0001, 0x0003");
        }

        return 0;

    }
        
    if(logging) add_log("there is no subformat, as format is not WAVE_FORMAT_EXTENSIBLE");    
    return 0;
    
}

bool waveconfig::set_channel_mask(uint32_t mask){
    channelMask = mask;
    return 1;
}

bool waveconfig::config(
        uint16_t format_,
        uint16_t channel_amount_,
        uint16_t sample_size_,
        uint32_t frame_rate_,
        uint16_t subformat_,
        uint32_t mask_){
    
    bool ret = 1;

    ret &= set_format(format_);
    ret &= set_channel_amount(channel_amount_);
    ret &= set_sample_bitsize(sample_size_);
    ret &= set_frame_rate(frame_rate_);

    if(format == EXTENSIBLE){
        ret &= set_subformat(subformat_);
        ret &= set_channel_mask(mask_);
        validSampleBits = sampleBits;
    }

    return ret;
}

bool waveconfig::copy_config(waveconfig *other){

    format = other->format;
    channels = other->channels;
    frameSize = other->frameSize;
    sampleBits = other->sampleBits;
    sampleSize = other->sampleSize;
    extensionSize = other->extensionSize;

    formatSize = other->formatSize;
    frameRate = other->frameRate;
    byteRate = other->byteRate;

    validSampleBits = other->validSampleBits;
    subformat = other->subformat;

    channelMask = other->channelMask;

    return 1;
}

uint32_t waveconfig::sample_amount(){
    return dataSize/sampleSize;
}

uint32_t waveconfig::frame_amount(){
    return dataSize/frameSize;
}

std::vector<uint32_t> waveconfig::get_config(){
    return {
        format,
        channels,
        sampleBits,
        frameRate,
        subformat,
        channelMask
    };
}

