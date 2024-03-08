#include "stereo.h"
void stereo_mode0(){
	
	/*
	while(true){
	
	std::cin.read(reinterpret_cast<char*>(IQ_buf.data()), 2*block_size);

		while(sample_num < 2*block_size){
			iq_sample = ((float)IQ_buf[sample_num]-128.0)/128.0;
			IQ_index = sample_num & 0x01;
			(*IQ[IQ_index])[sample_num >> 1] = iq_sample;
			sample_num += 1;
		}
	sample_num = 0;
	
	}
	*/
	//std::cerr << "TODO! - To be implemented in future renditions.\n";
}

void stereo_mode1(){std::cerr << "TODO! - To be implemented in future renditions.\n";}

void stereo_mode2(){std::cerr << "TODO! - To be implemented in future renditions.\n";}

void stereo_mode3(){std::cerr << "TODO! - To be implemented in future renditions.\n";}
