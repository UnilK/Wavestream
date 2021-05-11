#include "wavestream.h"
#include "wave_dialog.h"

const uint16_t EXTENSIBLE = 0xfffe, PCM = 0x0001, IEEE = 0x0003;

waveconfig::waveconfig(){}

waveconfig::waveconfig(uint16_t format, uint16_t channel_amount, uint16_t sample_size,
		uint32_t frame_rate, uint16_t subformat, uint32_t mask){	
	this->config(format, channel_amount, sample_size, frame_rate, subformat, mask);
}

waveconfig::waveconfig(waveconfig *other){
	this->copy_config(other);
}

void waveconfig::add_log(std::string message){
	this->log.push_back(message);
	if(this->log.size() >= 200){
		for(int i=0; i<100; i++) this->log[i] = this->log[i+100];
		this->log.resize(100);
	}
}

std::vector<std::string> waveconfig::get_log(){
	return this->log;
}



bool waveconfig::set_format(uint16_t format_){
	switch(format_){
		case PCM:
			this->formatSize = 16;
			this->extensionSize = 0;
			this->format = format_;
			return 1;
		case IEEE:
			this->formatSize = 18;
			this->extensionSize = 0;
			this->format = format_;
			return 1;
		case EXTENSIBLE:
			this->formatSize = 40;
			this->extensionSize = 22;
			this->format = format_;
			return 1;
	}
	
	this->add_log("format "+std::to_string(format_)
			+" not recognized. Known formats are 0x0001, 0x0003 and 0xfffe");
	return 0;
}

bool waveconfig::set_channel_amount(uint16_t channels_){
	
	if(channels_ > 18){
		this->add_log(std::to_string(channels_)+" is too many channels. Max 18.");
		return 0;
	}

	this->channels = channels_;
	this->frameSize = this->channels*this->sampleSize;
	this->byteRate = this->frameRate*this->frameSize;
	
	return 1;
}

bool waveconfig::set_sample_bitsize(uint16_t sampleBits_){
	
	if(sampleBits_%8){
		this->add_log("sample "+std::to_string(sampleBits_)
				+" size not supported, must be a multiple of 8.");
		return 0;
	}

	this->sampleBits = sampleBits_;
	this->sampleSize = this->sampleBits/8;
	this->frameSize = this->channels*this->sampleSize;
	this->byteRate = this->frameRate*this->frameSize;

	return 1;
}

bool waveconfig::set_frame_rate(uint32_t frameRate_){
	
	this->frameRate = frameRate_;
	this->byteRate = this->frameRate*this->frameSize;

	return 1;
}

bool waveconfig::set_subformat(uint16_t subformat_){

	if(this->format == EXTENSIBLE){
		
		switch(subformat_){
			case PCM:
				this->subformat = subformat_;
				return 1;
			case IEEE:
				this->subformat = subformat_;
				return 1;
		}
	
		this->add_log("Subormat "+std::to_string(subformat_)
				+" not recognized. Known formats are 0x0001, 0x0003");
		return 0;

	}
		
	this->add_log("there is no subformat, as format is not WAVE_FORMAT_EXTENSIBLE");	
	return 0;
	
}

bool waveconfig::set_channel_mask(uint32_t mask){
	this->channelMask = mask;
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

	ret &= this->set_format(format_);
	ret &= this->set_channel_amount(channel_amount_);
	ret &= this->set_sample_bitsize(sample_size_);
	ret &= this->set_frame_rate(frame_rate_);

	if(this->format == EXTENSIBLE){
		ret &= this->set_subformat(subformat_);
		ret &= this->set_channel_mask(mask_);
		this->validSampleBits = this->sampleBits;
	}

	return ret;
}

bool waveconfig::copy_config(waveconfig *other){

	this->format = other->format;
	this->channels = other->channels;
	this->frameSize	= other->frameSize;
	this->sampleBits = other->sampleBits;
	this->sampleSize = other->sampleSize;
	this->extensionSize = other->extensionSize;

	this->fileSize = other->fileSize;
	this->formatSize = other->formatSize;
	this->frameRate = other->frameRate;
	this->byteRate = other->byteRate;
	this->dataSize = other->dataSize;

	this->validSampleBits = other->validSampleBits;
	this->subformat = other->subformat;

	this->channelMask = other->channelMask;

	return 1;
}
