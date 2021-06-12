#include "wavestream.h"
#include "wave_dialog.h"

uint16_t iwavestream::read_uint16(){
    char buff[2];
    this->wavFile.read(buff, 2);    
    return wave_dialog::listen_uint16(buff);
}

uint32_t iwavestream::read_uint32(){
    char buff[4];
    this->wavFile.read(buff, 4);    
    return wave_dialog::listen_uint32(buff);
}

bool iwavestream::handle_unexpected_chunk(){
    
    uint32_t chunkSize = this->read_uint32();
    char *buff = new char[chunkSize];
    wavFile.read(buff, chunkSize);
    delete buff;

    if(!this->wavFile.good()){
        this->add_log("error reading unexpected chunk of size "+std::to_string(chunkSize));
        return 0;
    }

    return 1;
}

bool iwavestream::compare_id(char *buff, std::string pattern){
    for(uint32_t i=0; i<pattern.size(); i++) if(pattern[i] != buff[i]) return 0;
    return 1;
}



iwavestream::iwavestream(){
    this->source = "";
    this->listen_data = NULL;
}

iwavestream::iwavestream(std::string source_){
    this->open(source_);
}

bool iwavestream::open(std::string source_){
    this->source = source_;
    this->wavFile.open(source_);
    return this->initialize();
}

bool iwavestream::close(){
    this->wavFile.close();
    return 1;
}

bool iwavestream::initialize(){

    if(!this->wavFile.good()){
        this->add_log("error reading file");
        return 0;
    }

    char buff4[4];
    
    this->wavFile.read(buff4, 4);
    
    if(!this->compare_id(buff4, "RIFF")){
        this->add_log("file is not RIFF format");
        return 0;
    }

    this->fileSize = this->read_uint32();

    this->wavFile.read(buff4, 4);

    if(!this->compare_id(buff4, "WAVE")){
        this->add_log("file is not WAVE format");
        return 0;
    }

    this->wavFile.read(buff4, 4);

    while(!this->compare_id(buff4, "fmt ")){
        this->add_log("unexpexted chunk, expected \"fmt \"");
        bool ok = this->handle_unexpected_chunk();
        if(!ok) return 0;
        this->wavFile.read(buff4, 4);
    }
    
    this->formatSize = this->read_uint32();
    this->format = this->read_uint16();
    this->channels = this->read_uint16();
    this->frameRate = this->read_uint32();
    this->byteRate = this->read_uint32();
    this->frameSize = this->read_uint16();
    this->sampleBits = this->read_uint16();
    this->sampleSize = this->sampleBits/8;

    if(this->formatSize >= 18){
        this->extensionSize = this->read_uint16();
    }

    if(this->formatSize == 40){
        
        this->validSampleBits = this->read_uint16();
        this->channelMask = this->read_uint32();

        this->wavFile.read(this->GUID, 16);
        this->subformat = wave_dialog::listen_uint16(this->GUID);
    }

    if(this->format == 0xfffe){ 
        this->listen_data = wave_dialog::resolve_listener(this->subformat, this->validSampleBits);
        if(this->listen_data == NULL){
            this->add_log("format 0xfffe with subformat "+
                    std::to_string(this->subformat)+" "+std::to_string(this->validSampleBits)
                    +" is not supported.");
            return 0;
        }
    } else {

        this->subformat = this->format;             // this enables handling all files
        this->validSampleBits = this->sampleBits;   // as 0xfffe WAVE_FORMAT_EXTENSIBLE

        this->listen_data = wave_dialog::resolve_listener(this->format, this->sampleBits);
        if(this->listen_data == NULL){
            this->add_log("format "+std::to_string(this->format)
                    +" "+std::to_string(this->sampleBits)+" is not supported.");
            return 0;
        }
    }
    
    this->wavFile.read(buff4, 4);

    while(!this->compare_id(buff4, "data")){
        if(!this->compare_id(buff4, "fact")) this->add_log("unexpected chunk, expected \"data\"");
        bool ok = this->handle_unexpected_chunk();
        if(!ok) return 0;
        this->wavFile.read(buff4, 4);
    }

    this->dataSize = this->read_uint32();

    this->dataBegin = wavFile.tellg();

    this->add_log(
            "file initialized with:\nformat: "+std::to_string(this->format)
            +"\nsubformat (if format is 0xfffe): "+std::to_string(this->subformat)
            +"\nchannels: "+std::to_string(this->channels)
            +"\nsample size: "+std::to_string(this->validSampleBits)
            +"\nframe rate: "+std::to_string(this->frameRate)
            +"\ndata amount: "+std::to_string(this->dataSize));

    return 1;
}

bool iwavestream::read_frames(std::vector<float> *waves, uint32_t amount){

    if(!this->wavFile.good()){
        this->add_log("error reading file");
        return 0;
    }

    uint32_t readAmount = amount;
    
    int64_t probeSize = (int64_t)amount*this->frameSize+this->wavFile.tellg()-this->dataBegin;

    if(probeSize > this->dataSize){
        readAmount = amount - (probeSize-this->dataSize)/this->frameSize;
        this->add_log(
                "could only read "+std::to_string(readAmount)
                +" frames as end end of file was reached.");
    }

    uint32_t buffz = readAmount*this->frameSize;
    char *buff = new char[buffz];
    
    uint32_t bsize = waves->size();

    readAmount *= this->channels;

    waves->resize(bsize+amount, 0);
    
    this->wavFile.read(buff, buffz);

    for(uint32_t i=0; i<readAmount; i++){
        (*waves)[bsize+i] = this->listen_data(buff+this->sampleSize*i);
    }
    
    if(!this->wavFile){
        this->add_log("error reading file");
        return 0;
    }

    if(this->wavFile.eof() || (uint32_t)this->wavFile.tellg()-this->dataBegin >= this->dataSize){
        this->add_log("end of file reached");
        return 0;
    }

    return 1;
}

bool iwavestream::read_frames(float *waves, uint32_t amount){

    if(!this->wavFile.good()){
        this->add_log("error reading file");
        return 0;
    }

    uint32_t readAmount = amount;
    
    int64_t probeSize = (int64_t)amount*this->frameSize+this->wavFile.tellg()-this->dataBegin;

    if(probeSize > this->dataSize){
        readAmount = amount - (probeSize-this->dataSize)/this->frameSize;
        this->add_log(
                "could only read "+std::to_string(readAmount)
                +" frames as end end of file was reached.");
    }

    uint32_t buffz = readAmount*this->frameSize;
    char *buff = new char[buffz];
    
    readAmount *= this->channels;
    
    this->wavFile.read(buff, buffz);

    for(uint32_t i=0; i<readAmount; i++){
        waves[i] = this->listen_data(buff+this->sampleSize*i);
    }
    
    if(!this->wavFile){
        this->add_log("error reading file");
        return 0;
    }

    if(this->wavFile.eof() || (uint32_t)this->wavFile.tellg()-this->dataBegin >= this->dataSize){
        this->add_log("end of file reached");
        return 0;
    }

    return 1;
}

bool iwavestream::read_frames(std::vector<float> *waves, uint32_t beginFrame, uint32_t amount){
    
    if((int64_t)beginFrame*this->sampleSize > this->dataSize){
        this->add_log("couldn't read frames, beginFrame is out of bounds.");
        return 0;
    }

    this->wavFile.seekg(this->dataBegin+beginFrame*this->sampleSize);
    return this->read_frames(waves, amount);
}

bool iwavestream::read_frames(float *waves, uint32_t beginFrame, uint32_t amount){
    
    if((int64_t)beginFrame*this->sampleSize > this->dataSize){
        this->add_log("couldn't read frames, beginFrame is out of bounds.");
        return 0;
    }

    this->wavFile.seekg(this->dataBegin+beginFrame*this->sampleSize);
    return this->read_frames(waves, amount);
}

bool iwavestream::read_file(std::vector<float> *waves){
    this->wavFile.seekg(this->dataBegin);
    return this->read_frames(waves, this->dataSize/this->frameSize);
}

bool iwavestream::read_file(float *waves){
    this->wavFile.seekg(this->dataBegin);
    return this->read_frames(waves, this->dataSize/this->frameSize);
}

