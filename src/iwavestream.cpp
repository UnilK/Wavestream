#include "wavestream.h"
#include "wave_dialog.h"
#include "constants.h"

uint16_t iwavestream::read_uint16(){
    char buff[2];
    wavFile.read(buff, 2);    
    return wave_dialog::listen_uint16(buff);
}

uint32_t iwavestream::read_uint32(){
    char buff[4];
    wavFile.read(buff, 4);    
    return wave_dialog::listen_uint32(buff);
}

bool iwavestream::handle_unexpected_chunk(){
    
    uint32_t chunkSize = read_uint32();
    char *buff = new char[chunkSize];
    wavFile.read(buff, chunkSize);
    delete[] buff;

    if(!wavFile.good()){
        add_log("error reading unexpected chunk of size "+std::to_string(chunkSize));
        return 0;
    }

    return 1;
}

bool iwavestream::compare_id(char *buff, std::string pattern){
    for(uint32_t i=0; i<pattern.size(); i++) if(pattern[i] != buff[i]) return 0;
    return 1;
}



iwavestream::iwavestream(){
    source = "";
    listen_data = NULL;
}

iwavestream::iwavestream(std::string source_){
    open(source_);
}

bool iwavestream::open(std::string source_){
    source = source_;
    wavFile.open(source_);
    return initialize();
}

bool iwavestream::close(){
    wavFile.close();
    return 1;
}

bool iwavestream::initialize(){

    if(!wavFile.good()){
        add_log("error reading file");
        return 0;
    }

    char buff4[4];
    
    wavFile.read(buff4, 4);
    
    if(!compare_id(buff4, "RIFF")){
        add_log("file is not RIFF format");
        return 0;
    }

    fileSize = read_uint32();

    wavFile.read(buff4, 4);

    if(!compare_id(buff4, "WAVE")){
        add_log("file is not WAVE format");
        return 0;
    }

    wavFile.read(buff4, 4);

    while(!compare_id(buff4, "fmt ")){
        add_log("unexpexted chunk, expected \"fmt \"");
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
        listen_data = wave_dialog::resolve_listener(subformat, validSampleBits);
        if(listen_data == NULL){
            add_log("format 0xfffe with subformat "+
                    std::to_string(subformat)+" "+std::to_string(validSampleBits)
                    +" is not supported.");
            return 0;
        }
    } else {

        subformat = format;             // this should enable handling all files
        validSampleBits = sampleBits;   // as 0xfffe WAVE_FORMAT_EXTENSIBLE

        listen_data = wave_dialog::resolve_listener(format, sampleBits);
        if(listen_data == NULL){
            add_log("format "+std::to_string(format)
                    +" "+std::to_string(sampleBits)+" is not supported.");
            return 0;
        }
    }
    
    wavFile.read(buff4, 4);

    while(!compare_id(buff4, "data")){
        if(!compare_id(buff4, "fact")) add_log("unexpected chunk, expected \"data\"");
        bool ok = handle_unexpected_chunk();
        if(!ok) return 0;
        wavFile.read(buff4, 4);
    }

    dataSize = read_uint32();

    dataBegin = wavFile.tellg();

    add_log(
            "file initialized with:\nformat: "+std::to_string(format)
            +"\nsubformat (if format is 0xfffe): "+std::to_string(subformat)
            +"\nchannels: "+std::to_string(channels)
            +"\nsample size: "+std::to_string(validSampleBits)
            +"\nframe rate: "+std::to_string(frameRate)
            +"\ndata amount: "+std::to_string(dataSize));

    return 1;
}

uint32_t iwavestream::read_samples(std::vector<float> &waves, uint32_t amount){

    if(!wavFile.good()){
        add_log("error reading file");
        return 0;
    }

    int64_t bsize = waves.size();
    waves.resize(bsize+amount, 0);

    return read_samples(waves.data()+bsize, amount);
}

uint32_t iwavestream::read_samples(float *waves, uint32_t amount){

    if(!wavFile.good()){
        add_log("error reading file");
        return 0;
    }

    uint32_t readAmount = amount;
    
    int64_t probeSize = (int64_t)amount*sampleSize+wavFile.tellg()-dataBegin;

    if(probeSize > (int64_t)dataSize){
        readAmount = amount - (probeSize-dataSize)/sampleSize;
        add_log(
                "could only read "+std::to_string(readAmount)
                +" frames as end end of file was reached.");
    }

    int64_t buffz = readAmount*sampleSize;

    char *buff = new char[buffz];
    
    wavFile.read(buff, buffz);

    for(uint32_t i=0; i<readAmount; i++){
        waves[i] = listen_data(buff+i*sampleSize);
    }

    delete[] buff;
    
    if(!wavFile){
        add_log("error reading file");
        return 0;
    }

    if(wavFile.eof() || (uint32_t)wavFile.tellg()-dataBegin >= dataSize){
        add_log("end of file reached");
        return readAmount;
    }

    return readAmount;
}

uint32_t iwavestream::read_samples(std::vector<float> &waves, uint32_t beginSample, uint32_t amount){
    
    if((int64_t)beginSample*sampleSize > (int64_t)dataSize){
        add_log("couldn't read frames, beginSample is out of bounds.");
        return 0;
    }

    wavFile.seekg(dataBegin+beginSample*sampleSize);
    return read_samples(waves, amount);
}

uint32_t iwavestream::read_samples(float *waves, uint32_t beginSample, uint32_t amount){
    
    if((int64_t)beginSample*sampleSize > (int64_t)dataSize){
        add_log("couldn't read frames, beginSample is out of bounds.");
        return 0;
    }

    wavFile.seekg(dataBegin+beginSample*sampleSize);
    return read_samples(waves, amount);
}

uint32_t iwavestream::read_file(std::vector<float> &waves){
    wavFile.seekg(dataBegin);
    return read_samples(waves, dataSize/sampleSize);
}

uint32_t iwavestream::read_file(float *waves){
    wavFile.seekg(dataBegin);
    return read_samples(waves, dataSize/sampleSize);
}

