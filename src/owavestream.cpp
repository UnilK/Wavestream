#include "wavestream.h"
#include "wave_dialog.h"

void owavestream::write_uint16(uint16_t num){
    char buff[2];
    wave_dialog::say_uint16(num, buff);
    this->wavFile.write(buff, 2);   
}

void owavestream::write_uint32(uint32_t num){
    char buff[4];
    wave_dialog::say_uint32(num, buff);
    this->wavFile.write(buff, 4);   
}



owavestream::owavestream(){
    this->outSource = "";
    this->speak_data = NULL;
}

owavestream::owavestream(std::string outSource_, uint16_t format, uint16_t channel_amount,
        uint16_t sample_size, uint32_t frame_rate, uint16_t subformat, uint32_t mask){
    this->speak_data = NULL;
    this->open(outSource_);
    this->config(format, channel_amount, sample_size, frame_rate, subformat, mask);
    this->initialize();
}

owavestream::owavestream(std::string outSource_, waveconfig *other){
    this->speak_data = NULL;
    this->open(outSource_);
    this->copy_config(other);
    this->initialize();
}

bool owavestream::open(std::string outSource_){
    this->outSource = outSource_;
    this->wavFile.open(this->outSource);
    if(!this->wavFile.good()) this->add_log("error opening file");
    return this->wavFile.good();
}

bool owavestream::close(){

    this->fileSize = 12+this->formatSize+8+this->dataSize+(this->dataSize&1);
    if(this->format != 0x0001) this->fileSize += 12;

    char zeropad[64] = {0};
    while((this->dataSize/this->sampleSize) % this->channels){
        this->wavFile.write(zeropad, this->sampleSize);
        this->dataSize += this->sampleSize;
    }

    if(this->dataSize&1){
        // add pad byte if needed.
        this->wavFile.write(zeropad, 1);
    }

    this->wavFile.seekp(this->chunkSizePos);
    this->write_uint32(this->fileSize);

    this->wavFile.seekp(this->dataSizePos);
    this->write_uint32(this->dataSize);

    if(this->uselessPos){
        this->wavFile.seekp(this->uselessPos);
        this->write_uint32(this->dataSize/this->sampleSize);
    }

    if(!this->wavFile){
        this->add_log("error writing file while closing");
        this->wavFile.close();
        return 0;
    }

    this->wavFile.close();
    
    this->add_log(
            "file closed with total size "+std::to_string(this->fileSize)
            +" and data size "+std::to_string(this->dataSize));
    
    return 1;
}

const char EXTENSIBLE_GUID[15] = "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9b\x71";

bool owavestream::initialize(){
    
    if(!this->wavFile.good()){
        this->add_log("error writing file while initializing");
        return 0;
    }

    this->wavFile.seekp(0);

    if(this->format == 0xfffe){ 
        this->speak_data = wave_dialog::resolve_speaker(this->subformat, this->validSampleBits);
        if(this->speak_data == NULL){
            this->add_log("format 0xfffe with subformat "+
                    std::to_string(this->subformat)+" "+std::to_string(this->validSampleBits)
                    +" is not supported.");
            return 0;
        }
    } else {

        this->subformat = this->format;
        this->validSampleBits = this->sampleBits;

        this->speak_data = wave_dialog::resolve_speaker(this->format, this->sampleBits);
        if(this->speak_data == NULL){
            this->add_log("format "+std::to_string(this->format)
                    +" "+std::to_string(this->sampleBits)+" is not supported.");
            return 0;
        }
    }

    char riffid[] = "RIFF", waveid[] = "WAVE", fmtid[] = "fmt ", dataid[] = "data";
    
    this->wavFile.write(riffid, 4);

    // save the chunk size position for closing the file
    this->chunkSizePos = this->wavFile.tellp();
    this->write_uint32(this->fileSize);

    this->wavFile.write(waveid, 4);
    
    this->wavFile.write(fmtid, 4);
    this->write_uint32(this->formatSize);
    this->write_uint16(this->format);
    this->write_uint16(this->channels);
    this->write_uint32(this->frameRate);
    this->write_uint32(this->byteRate);
    this->write_uint16(this->frameSize);
    this->write_uint16(this->sampleBits);

    if(this->formatSize >= 18){
        this->write_uint16(this->extensionSize);
    }

    if(this->formatSize == 40){

        uint16_t channel_count = 0;
        for(uint32_t i=0; i<18; i++) channel_count += (this->channelMask>>i)&1;

        if(this->channels != channel_count){
            this->add_log("error: channel count in mask doesn't match channel amount");
            return 0;
        }

        this->write_uint16(this->validSampleBits);
        this->write_uint32(this->channelMask);
        this->write_uint16(this->subformat);
        this->wavFile.write(EXTENSIBLE_GUID, 14);

    }

    if(this->format != 0x0001){ 
        
        // weird standard.

        uint32_t chunkSize = 4, uselessInfo = 0;

        char factid[] = "fact";
        this->wavFile.write(factid, 4);
        this->write_uint32(chunkSize);

        this->uselessPos = this->wavFile.tellp();
        this->write_uint32(uselessInfo);

    }

    this->wavFile.write(dataid, 4);

    // save the data size position for closing the file
    this->dataSizePos = this->wavFile.tellp();
    this->write_uint32(this->dataSize); 
    
    this->add_log(
            "file initialized with:\nformat: "+std::to_string(this->format)
            +"\nsubformat (if format is 0xfffe): "+std::to_string(this->subformat)
            +"\nchannels: "+std::to_string(this->channels)
            +"\nsample size: "+std::to_string(this->validSampleBits)
            +"\nframe rate: "+std::to_string(this->frameRate));
    

    return 1;
}

bool owavestream::write_samples(std::vector<float> &waves){

    uint32_t wsize = waves.size();

    if((int64_t)this->dataSize+wsize+72 >= 1ll<<32){
        this->add_log("can't write samples, file would be too large");
        return 0;
    }

    if(wsize == 0) return 1;

    this->dataSize += wsize*this->sampleSize;
    
    char *buff = new char[wsize*this->sampleSize];

    for(uint32_t i=0; i<wsize; i++){
        this->speak_data(waves[i], buff+i*this->sampleSize);
    }

    this->wavFile.write(buff, wsize*this->sampleSize);

    delete[] buff;

    if(!this->wavFile.good()){
        this->add_log("error writing file");
        return 0;
    }

    return 1;
}

bool owavestream::write_samples(float *waves, uint32_t amount){

    uint32_t wsize = amount;

    if((int64_t)this->dataSize+wsize+72 >= 1ll<<32){
        this->add_log("can't write samples, file would be too large");
        return 0;
    }

    if(wsize == 0) return 1;

    this->dataSize += wsize*this->sampleSize;
    
    char *buff = new char[wsize*this->sampleSize];

    for(uint32_t i=0; i<wsize; i++){
        this->speak_data(waves[i], buff+i*this->sampleSize);
    }

    this->wavFile.write(buff, wsize*this->sampleSize);

    delete[] buff;

    if(!this->wavFile.good()){
        this->add_log("error writing file");
        return 0;
    }

    return 1;
}

bool owavestream::write_file(std::vector<float> &waves){
    if(!this->write_samples(waves)) return 0;
    return this->close();
}

bool owavestream::write_file(float *waves, uint32_t amount){
    if(!this->write_samples(waves, amount)) return 0;
    return this->close();
}

