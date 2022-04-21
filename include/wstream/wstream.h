#ifndef WAVESTREAM_H
#define WAVESTREAM_H

#include <cstdint>
#include <fstream>
#include <vector>
#include <string>

class waveconfig{

protected:

    char GUID[16];  // the first 2 bytes are the subformat.
    std::vector<std::string> log;
    void add_log(std::string message);

    /*
        I'm following the documentation on the wave format found on this site:
        http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    */

    // standard info
    uint16_t
        format = 0,         // 0x0001 (PCM), 0x0003 (IEEE) or 0xfffe (EXTENSIBLE)
        channels = 0,       // 1 - 18 of channels
        frameSize = 0,      // 1 frame = (1 sample from each channel)
        sampleBits = 0,     // number of (used?) bits in a sample. Only multiples of 8 are supported.
        sampleSize = 0,     // sample size in bytes.
        extensionSize = 0;  // Useless, formatSize already has this info


    uint32_t
        fileSize = 0,       // (size of the whole file in bytes) - 8 
        formatSize = 0,     // format chunk size in bytes
        frameRate = 0,      // sampling rate = frames per second
        byteRate = 0,       // bytes per second
        dataSize = 0;       // data chunk size in bytes



    // info for extensible format:

    uint16_t 
        validSampleBits = 0,    // same as sampleBits, as only multiples of 8 are supported.
        subformat = 0;          // actual format of the wave format extensible.

    uint32_t
        channelMask = 0;    // bitmask of the speaker positions for the channels
    /*

        the positions in the mask are:

        0.   Front Left - FL,
        1.   Front Right - FR,
        2.   Front Center - FC
        3.   Low Frequency - LF
        4.   Back Left - BL
        5.   Back Right - BR
        6.   Front Left of Center - FLC
        7.   Front Right of Center - FRC
        8.   Back Center - BC
        9.   Side Left - SL
        10.  Side Right - SR
        11.  Top Center - TC
        12.  Top Front Left - TFL
        13.  Top Front Center - TFC
        14.  Top Front Right - TFR
        15.  Top Back Left - TBL
        16.  Top Back Center - TBC
        17.  Top Back Right - TBR

        mono uses FL                            000000000000000001
        stereo uses FL, FR                      000000000000000011
        5.1 uses FL, FR, FC, LF, BL, BR         000000000000111111 (probably)
        7.1 uses FL, FR, FC, LF, BL, BR, SL, SR 000000011000111111 (I guess)
        
    */

public:

    waveconfig();
    waveconfig(
            uint16_t format,
            uint16_t channel_amount,
            uint16_t sample_size,
            uint32_t frame_rate,
            uint16_t subformat = 0,
            uint32_t mask = 0);

    waveconfig(waveconfig *other);

    std::vector<std::string> get_log();
    bool logging = 0;

    // all functions returning a boolean will tell wether the function call was succesful.
    // If an error or something worth of notice happened, a message is added to the log.

    bool set_format(uint16_t);
    bool set_channel_amount(uint16_t);
    bool set_sample_bitsize(uint16_t);
    bool set_frame_rate(uint32_t);      // frames per second
    bool set_subformat(uint16_t);
    bool set_channel_mask(uint32_t);

    bool config(
            uint16_t format,
            uint16_t channel_amount,
            uint16_t sample_size,
            uint32_t frame_rate,
            uint16_t subformat = 0,
            uint32_t mask = 0);

    bool copy_config(waveconfig*);

    uint32_t get_sample_amount();   // data size in samples.
    uint32_t get_frame_amount();    // data size in frames.

    uint16_t get_format();
    uint16_t get_channel_amount();
    uint16_t get_sample_bitsize();
    uint32_t get_frame_rate();  
    uint16_t get_subformat();
    uint32_t get_channel_mask();

};



class iwstream : public waveconfig{

protected:
    
    std::string source;
    std::ifstream wavFile;
    uint32_t dataBegin;

    uint16_t read_uint16();
    uint32_t read_uint32();

    bool handle_unexpected_chunk();
    bool compare_id(char*, std::string);

    uint32_t datatype = 0;

public:
    
    iwstream();

    // also opens & initializes the the stream
    iwstream(std::string source_);

    // reads the wave file configuration.
    bool initialize();

    // opens & initializes the wave file.
    // iwstream must be opened before anything can be read.
    bool open(std::string source_);
    bool close();

    // tell & seek reading position.
    uint32_t tell();
    bool seek(uint32_t beginSample);

    // continue reading amount samples from the current position.
    // if end of file is reached, the rest of the values are assigned to 0.
    // for the vector overload, values are appended to the end of the vector.
    // returns the amount of samples read.
    uint32_t read_move(std::vector<float> &waves, uint32_t amount);
    uint32_t read_move(float *waves, uint32_t amount);
    std::vector<float> read_move(uint32_t amount);
    
    // read samples but dont move the file pointer forward
    uint32_t read_silent(std::vector<float> &waves, uint32_t amount);
    uint32_t read_silent(float *waves, uint32_t amount);
    std::vector<float> read_silent(uint32_t amount);

    // navigate to beginFrame & read from that point.
    uint32_t read_move(std::vector<float> &waves, uint32_t beginSample, uint32_t amount);
    uint32_t read_move(float *waves, uint32_t beginSample, uint32_t amount);
    std::vector<float> read_move(uint32_t beginSample, uint32_t amount);
    

    uint32_t read_silent(std::vector<float> &waves, uint32_t beginSample, uint32_t amount);
    uint32_t read_silent(float *waves, uint32_t beginSample, uint32_t amount);
    std::vector<float> read_silent(uint32_t beginSample, uint32_t amount);
    
    // navigate to begin of file and read all frames.
    uint32_t read_file(std::vector<float> &waves);
    uint32_t read_file(float *waves);
    std::vector<float> read_file();
        
};



class owstream : public waveconfig{

protected:

    uint32_t chunkSizePos = 4, dataSizePos = 0, uselessPos = 0;

    std::string outSource;
    std::ofstream wavFile;

    void write_uint16(uint16_t);
    void write_uint32(uint32_t);

    uint32_t datatype = 0;

public:

    owstream();
    
    // these constructors open & initialize the file
    owstream(
            std::string outSource_,
            uint16_t format,
            uint16_t channel_amount,
            uint16_t sample_size,
            uint32_t frame_rate,
            uint16_t subformat = 0,
            uint32_t mask = 0);

    owstream(std::string outSource_, waveconfig *other);

    // wave files must be initialized before anything can be written on them.
    bool initialize();

    // opens a wave file for writing
    bool open(std::string outSource_);

    // a file must be closed so that the data will written correctly.
    bool close();

    // appends samples at the end of the file. SAMPLES, not FRAMES.
    bool write_move(const std::vector<float> &waves);
    bool write_move(const float *waves, uint32_t amount);

    // appends waves at the end of the file and closes it
    bool write_file(const std::vector<float> &waves);
    bool write_file(const float *waves, uint32_t amount);

};

#endif
