#include "wstream/wstream.h"
#include "wstream/wave_dialog.h"
#include "wstream/constants.h"

uint16_t iwstream::read_uint16(){
    char buff[2];
    wavFile.read(buff, 2);    
    return wave_dialog::listen_uint16(buff);
}

uint32_t iwstream::read_uint32(){
    char buff[4];
    wavFile.read(buff, 4);    
    return wave_dialog::listen_uint32(buff);
}

bool iwstream::handle_unexpected_chunk(){
    
    uint32_t chunkSize = read_uint32();
    char *buff = new char[chunkSize];
    wavFile.read(buff, chunkSize);

    if(!wavFile.good()){
        if(logging) add_log("error reading unexpected chunk of size "+std::to_string(chunkSize));
        return 0;
    }

    delete[] buff;

    return 1;
}

bool iwstream::compare_id(char *buff, std::string pattern){
    for(uint32_t i=0; i<pattern.size(); i++) if(pattern[i] != buff[i]) return 0;
    return 1;
}



iwstream::iwstream(){
    source = "";
}

iwstream::iwstream(std::string source_){
    open(source_);
}

bool iwstream::open(std::string source_){
    source = source_;
    wavFile.open(source_);
    return initialize();
}

bool iwstream::close(){
    wavFile.close();
    return 1;
}

bool iwstream::initialize(){

    if(!wavFile.good()){
        if(logging) add_log("error reading file");
        return 0;
    }

    char buff4[4];
    
    wavFile.read(buff4, 4);
    
    if(!compare_id(buff4, "RIFF")){
        if(logging) add_log("file is not RIFF format");
        return 0;
    }

    fileSize = read_uint32();

    wavFile.read(buff4, 4);

    if(!compare_id(buff4, "WAVE")){
        if(logging) add_log("file is not WAVE format");
        return 0;
    }

    wavFile.read(buff4, 4);

    while(!compare_id(buff4, "fmt ")){
        if(logging) add_log("unexpexted chunk, expected \"fmt \"");
        bool ok = handle_unexpected_chunk();
        if(!ok) return 0;
        wavFile.read(buff4, 4);
    }
    
    formatSize = read_uint32();
    format = read_uint16();
    channels = read_uint16();
    frameRate = read_uint32();
    byteRate = read_uint32();
    frameSize = read_uint16();
    sampleBits = read_uint16();
    sampleSize = sampleBits/8;

    if(formatSize >= 18){
        extensionSize = read_uint16();
    }

    if(formatSize == 40){
        
        validSampleBits = read_uint16();
        channelMask = read_uint32();

        wavFile.read(GUID, 16);
        subformat = wave_dialog::listen_uint16(GUID);
    }

    if(format == EXTENSIBLE){ 
        datatype = wave_dialog::resolve_dialog(subformat, validSampleBits);
        if(!datatype){
            if(logging){
                add_log("format 0xfffe with subformat "+
                    std::to_string(subformat)+" "+std::to_string(validSampleBits)
                    +" is not supported.");
            }
            return 0;
        }
    } else {

        subformat = format;             // this should enable handling all files
        validSampleBits = sampleBits;   // as 0xfffe WAVE_FORMAT_EXTENSIBLE

        datatype = wave_dialog::resolve_dialog(format, sampleBits);
        if(!datatype){
            if(logging){
                add_log("format "+std::to_string(format)
                    +" "+std::to_string(sampleBits)+" is not supported.");
            }
            return 0;
        }
    }
    
    wavFile.read(buff4, 4);

    while(!compare_id(buff4, "data")){
        if(!compare_id(buff4, "fact") && logging) add_log("unexpected chunk, expected \"data\"");
        bool ok = handle_unexpected_chunk();
        if(!ok) return 0;
        wavFile.read(buff4, 4);
    }

    dataSize = read_uint32();

    dataBegin = wavFile.tellg();

    if(logging){
        add_log(
            "file initialized with:\nformat: "+std::to_string(format)
            +"\nsubformat (if format is 0xfffe): "+std::to_string(subformat)
            +"\nchannels: "+std::to_string(channels)
            +"\nsample size: "+std::to_string(validSampleBits)
            +"\nframe rate: "+std::to_string(frameRate)
            +"\ndata amount: "+std::to_string(dataSize));
    }

    return 1;
}

uint32_t iwstream::tell(){
    return ((uint32_t)wavFile.tellg() - dataBegin) / sampleSize;
}

bool iwstream::seek(uint32_t beginSample){

    if((int64_t)beginSample*sampleSize > (int64_t)dataSize){
        if(logging) add_log("couldn't move to position, sample is out of bounds.");
        return 0;
    }

    wavFile.seekg(dataBegin+beginSample*sampleSize);

    return 1;
}

uint32_t iwstream::read_move(std::vector<float> &waves, uint32_t amount){

    if(!wavFile.good()){
        if(logging) add_log("error reading file");
        return 0;
    }

    int64_t bsize = waves.size();
    waves.resize(bsize+amount, 0);

    return read_move(waves.data()+bsize, amount);
}

uint32_t iwstream::read_move(float *waves, uint32_t amount){

    if(!wavFile.good()){
        if(logging) add_log("error reading file");
        return 0;
    }

    uint32_t readAmount = amount;
    
    int64_t probeSize = (int64_t)amount*sampleSize+wavFile.tellg()-dataBegin;

    if(probeSize > (int64_t)dataSize){
        readAmount = amount - (probeSize-dataSize)/sampleSize;
        if(logging){
            add_log(
                "could only read "+std::to_string(readAmount)
                +" frames as end end of file was reached.");
        }
    }

    int64_t buffz = readAmount*sampleSize;

    char *buff = new char[buffz];
    
    wavFile.read(buff, buffz);

    switch(datatype){
        case wave_dialog::INT8_ID:
            for(uint32_t i=0; i<readAmount; i++){
                waves[i] = wave_dialog::listen_int8_as_float(buff+i*sampleSize);
            }
            break;
        case wave_dialog::INT16_ID:
            for(uint32_t i=0; i<readAmount; i++){
                waves[i] = wave_dialog::listen_int16_as_float(buff+i*sampleSize);
            }
            break;
        case wave_dialog::INT24_ID:
            for(uint32_t i=0; i<readAmount; i++){
                waves[i] = wave_dialog::listen_int24_as_float(buff+i*sampleSize);
            }
            break;
        case wave_dialog::INT32_ID:
            for(uint32_t i=0; i<readAmount; i++){
                waves[i] = wave_dialog::listen_int32_as_float(buff+i*sampleSize);
            }
            break;
        case wave_dialog::FLOAT32_ID:
            for(uint32_t i=0; i<readAmount; i++){
                waves[i] = wave_dialog::listen_int32_as_float(buff+i*sampleSize);
            }
            break;
        default:
            if(logging){
                add_log("file was initialized with unrecognized datatype\nand no data was read");
            }
    }
    
    if(!wavFile){
        if(logging) add_log("error reading file");
        return 0;
    }

    if(wavFile.eof() || (uint32_t)wavFile.tellg()-dataBegin >= dataSize){
        if(logging) add_log("end of file reached");
        return readAmount;
    }

    delete[] buff;

    return readAmount;
}

std::vector<float> iwstream::read_move(uint32_t amount){
    std::vector<float> waves;
    read_move(waves, amount);
    return waves;
}

uint32_t iwstream::read_silent(std::vector<float> &waves, uint32_t amount){
    uint32_t previous = tell();
    uint32_t num = read_move(waves, amount);
    seek(previous);
    return num;
}

uint32_t iwstream::read_silent(float *waves, uint32_t amount){
    uint32_t previous = tell();
    uint32_t num = read_move(waves, amount);
    seek(previous);
    return num;
}

std::vector<float> iwstream::read_silent(uint32_t amount){
    std::vector<float> waves;
    read_silent(waves, amount);
    return waves;
}

uint32_t iwstream::read_move(std::vector<float> &waves, uint32_t beginSample, uint32_t amount){
    if(!seek(beginSample)) return 0;
    return read_move(waves, amount);
}

uint32_t iwstream::read_move(float *waves, uint32_t beginSample, uint32_t amount){
    if(!seek(beginSample)) return 0;
    return read_move(waves, amount);
}

std::vector<float> iwstream::read_move(uint32_t beginSample, uint32_t amount){
    std::vector<float> waves;
    read_move(waves, beginSample, amount);
    return waves;
}

uint32_t iwstream::read_silent(std::vector<float> &waves, uint32_t beginSample, uint32_t amount){
    uint32_t previous = tell();
    uint32_t num = read_move(waves, beginSample, amount);
    seek(previous);
    return num;
}

uint32_t iwstream::read_silent(float *waves, uint32_t beginSample, uint32_t amount){
    uint32_t previous = tell();
    uint32_t num = read_move(waves, beginSample, amount);
    seek(previous);
    return num;
}

std::vector<float> iwstream::read_silent(uint32_t beginSample, uint32_t amount){
    std::vector<float> waves;
    read_silent(waves, beginSample, amount);
    return waves;
}

uint32_t iwstream::read_file(std::vector<float> &waves){
    wavFile.seekg(dataBegin);
    return read_move(waves, dataSize/sampleSize);
}

uint32_t iwstream::read_file(float *waves){
    wavFile.seekg(dataBegin);
    return read_move(waves, dataSize/sampleSize);
}

std::vector<float> iwstream::read_file(){
    std::vector<float> waves;
    read_file(waves);
    return waves;
}

