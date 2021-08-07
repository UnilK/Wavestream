#include "wavestream.h"
#include "wave_dialog.h"
#include "constants.h"

void owavestream::write_uint16(uint16_t num){
    char buff[2];
    wave_dialog::say_uint16(num, buff);
    wavFile.write(buff, 2);   
}

void owavestream::write_uint32(uint32_t num){
    char buff[4];
    wave_dialog::say_uint32(num, buff);
    wavFile.write(buff, 4);   
}



owavestream::owavestream(){
    outSource = "";
    speak_data = NULL;
}

owavestream::owavestream(std::string outSource_, uint16_t format, uint16_t channel_amount,
        uint16_t sample_size, uint32_t frame_rate, uint16_t subformat, uint32_t mask){
    speak_data = NULL;
    open(outSource_);
    config(format, channel_amount, sample_size, frame_rate, subformat, mask);
    initialize();
}

owavestream::owavestream(std::string outSource_, waveconfig *other){
    speak_data = NULL;
    open(outSource_);
    copy_config(other);
    initialize();
}

bool owavestream::open(std::string outSource_){
    outSource = outSource_;
    wavFile.open(outSource);
    if(!wavFile.good()) add_log("error opening file");
    return wavFile.good();
}

bool owavestream::close(){

    fileSize = 12+formatSize+8+dataSize+(dataSize&1);
    if(format != PCM) fileSize += 12;

    char zeropad[64] = {0};
    while((dataSize/sampleSize) % channels){
        wavFile.write(zeropad, sampleSize);
        dataSize += sampleSize;
    }

    if(dataSize&1){
        // add pad byte if needed.
        wavFile.write(zeropad, 1);
    }

    wavFile.seekp(chunkSizePos);
    write_uint32(fileSize);

    wavFile.seekp(dataSizePos);
    write_uint32(dataSize);

    if(uselessPos){
        wavFile.seekp(uselessPos);
        write_uint32(dataSize/sampleSize);
    }

    if(!wavFile){
        add_log("error writing file while closing");
        wavFile.close();
        return 0;
    }

    wavFile.close();
    
    add_log(
            "file closed with total size "+std::to_string(fileSize)
            +" and data size "+std::to_string(dataSize));
    
    return 1;
}

bool owavestream::initialize(){
    
    if(!wavFile.good()){
        add_log("error writing file while initializing");
        return 0;
    }

    wavFile.seekp(0);

    if(format == EXTENSIBLE){ 
        speak_data = wave_dialog::resolve_speaker(subformat, validSampleBits);
        if(speak_data == NULL){
            add_log("format 0xfffe with subformat "+
                    std::to_string(subformat)+" "+std::to_string(validSampleBits)
                    +" is not supported.");
            return 0;
        }
    } else {

        subformat = format;
        validSampleBits = sampleBits;

        speak_data = wave_dialog::resolve_speaker(format, sampleBits);
        if(speak_data == NULL){
            add_log("format "+std::to_string(format)
                    +" "+std::to_string(sampleBits)+" is not supported.");
            return 0;
        }
    }

    char riffid[] = "RIFF", waveid[] = "WAVE", fmtid[] = "fmt ", dataid[] = "data";
    
    wavFile.write(riffid, 4);

    // save the chunk size position for closing the file
    chunkSizePos = wavFile.tellp();
    write_uint32(fileSize);

    wavFile.write(waveid, 4);
    
    wavFile.write(fmtid, 4);
    write_uint32(formatSize);
    write_uint16(format);
    write_uint16(channels);
    write_uint32(frameRate);
    write_uint32(byteRate);
    write_uint16(frameSize);
    write_uint16(sampleBits);

    if(formatSize >= 18){
        write_uint16(extensionSize);
    }

    if(formatSize == 40){

        uint16_t channel_count = 0;
        for(uint32_t i=0; i<18; i++) channel_count += (channelMask>>i)&1;

        if(channels != channel_count){
            add_log("error: channel count in mask doesn't match channel amount");
            return 0;
        }

        write_uint16(validSampleBits);
        write_uint32(channelMask);
        write_uint16(subformat);
        wavFile.write(EXTENSIBLE_GUID, 14);

    }

    if(format != PCM){ 
        
        // weird standard.

        uint32_t chunkSize = 4, uselessInfo = 0;

        char factid[] = "fact";
        wavFile.write(factid, 4);
        write_uint32(chunkSize);

        uselessPos = wavFile.tellp();
        write_uint32(uselessInfo);

    }

    wavFile.write(dataid, 4);

    // save the data size position for closing the file
    dataSizePos = wavFile.tellp();
    write_uint32(dataSize); 
    
    add_log(
            "file initialized with:\nformat: "+std::to_string(format)
            +"\nsubformat (if format is 0xfffe): "+std::to_string(subformat)
            +"\nchannels: "+std::to_string(channels)
            +"\nsample size: "+std::to_string(validSampleBits)
            +"\nframe rate: "+std::to_string(frameRate));
    

    return 1;
}

bool owavestream::write_samples(std::vector<float> &waves){
    return write_samples(waves.data(), waves.size());
}

bool owavestream::write_samples(float *waves, uint32_t amount){

    if((int64_t)dataSize+amount+72 >= 1ll<<32){
        add_log("can't write samples, file would be too large");
        return 0;
    }

    if(amount == 0) return 1;

    dataSize += amount*sampleSize;
    
    char *buff = new char[amount*sampleSize];

    for(uint32_t i=0; i<amount; i++){
        speak_data(waves[i], buff+i*sampleSize);
    }

    wavFile.write(buff, amount*sampleSize);

    delete[] buff;

    if(!wavFile.good()){
        add_log("error writing file");
        return 0;
    }

    return 1;
}

bool owavestream::write_file(std::vector<float> &waves){
    if(!write_samples(waves)) return 0;
    return close();
}

bool owavestream::write_file(float *waves, uint32_t amount){
    if(!write_samples(waves, amount)) return 0;
    return close();
}

