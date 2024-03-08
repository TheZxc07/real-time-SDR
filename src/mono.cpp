
#include "mono.h"
#include "filter.h"
#include "demod.h"
#include "args.h"
#include "threadsafequeue.h"
#include <bitset>

void mono_mode0(args* p){
	
	std::vector<float> audio_h;
	std::vector<float> rf_h;
	
	int rf_Fs = 2.4e6;
	int rf_Fc = 100e3;
	unsigned short int rf_taps = 101;
	int rf_decim = 10;
	int audio_decim = 5;
	int audio_Fs = 240e3;
	int audio_Fc = 16e3;

	int block_size = 1024 * rf_decim * audio_decim;
	int block_count = 0;
	
	impulseResponseLPF(rf_Fs, rf_Fc, rf_taps, rf_h);
	impulseResponseLPF(audio_Fs, audio_Fc, rf_taps, audio_h);
	
	float iq_sample;
	int sample_num = 0;
	int IQ_index;
	
	uint8_t x;
		
	std::vector<uint8_t> IQ_buf = std::vector<uint8_t>(2*block_size);
	std::vector<float> I = std::vector<float>(block_size);
	std::vector<float> Q = std::vector<float>(block_size);
	std::vector<float> I_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> Q_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> filt_IQ = std::vector<float>(block_size);
	std::vector<float> state_I = std::vector<float>(rf_h.size()-1);
	std::vector<float> state_Q = std::vector<float>(rf_h.size()-1);
	state_I.clear();
	state_Q.clear();
	std::vector<float>* fm_demod;
	float prev_I = 0, prev_Q = 0;
	std::vector<float> audio_filt = std::vector<float>(block_size/rf_decim);
	std::vector<short> audio = std::vector<short>((block_size/rf_decim)/audio_decim);
	std::vector<float> state_audio = std::vector<float>(audio_h.size()-1);
	std::vector<float>* IQ[] = {&I, &Q};
	std::string sample_string;
	uint8_t byte;

	while(true){
		
		/*
		std::cin.read(reinterpret_cast<char*>(IQ_buf.data()), 2*block_size);

		while(sample_num < 2*block_size){
			iq_sample = ((float)IQ_buf[sample_num]-128.0)/128.0;
			IQ_index = sample_num & 0x01;
			(*IQ[IQ_index])[sample_num >> 1] = iq_sample;
			sample_num += 1;
		}
		sample_num = 0;
		
		convolveFIR(I_ds, I, rf_h, state_I, rf_decim);
	
		convolveFIR(Q_ds, Q, rf_h, state_Q, rf_decim);

		fmDemodNoArctan(I_ds, Q_ds, prev_I, prev_Q, fm_demod);
		*/
		while(!(p->queue.empty())){
			std::cerr << "Processing block: " << block_count << "\n";
			p->queue.wait_and_pop(fm_demod);
	
			convolveFIR(audio_filt, *fm_demod, audio_h, state_audio, audio_decim);
			
			for (int i = 0; i < audio.size(); i++){
				audio[i] = (short int)(16384*audio_filt[i]);
			}
			
		
			fwrite(&audio[0], sizeof(short int), audio.size(), stdout); 
			
			
			delete fm_demod;
			block_count++;
			
		}
		
		
		
		//for (uint i = 0; i < audio_filt.size(); i+=audio_decim){
			//std::cout << (signed short int)(16384*audio_filt[i]);
			//std::cerr << (signed short int)(16384*audio_filt[i]) << std::endl;
			
			//if (i == 10*audio_decim){ exit(1);
			//}
		//}
		//std::cerr << block_count << "\n";
		//block_count += 1;
		
	}
}

void mono_mode1(){
		
	std::vector<float> audio_h;
	std::vector<float> rf_h;
	
	int rf_Fs = 1.44e6;
	int rf_Fc = 100e3;
	unsigned short int rf_taps = 101;
	int rf_decim = 4;
	int audio_decim = 9;
	int audio_Fs = 360e3;
	int audio_Fc = 16e3;

	int block_size = 1024 * rf_decim * audio_decim;
	int block_count = 0;
	
	impulseResponseLPF(rf_Fs, rf_Fc, rf_taps, rf_h);
	impulseResponseLPF(audio_Fs, audio_Fc, rf_taps, audio_h);
	
	float iq_sample;
	int sample_num = 0;
	int IQ_index;
	
	uint8_t x;
		
	std::vector<uint8_t> IQ_buf = std::vector<uint8_t>(2*block_size);
	std::vector<float> I = std::vector<float>(block_size);
	std::vector<float> Q = std::vector<float>(block_size);
	std::vector<float> I_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> Q_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> filt_IQ = std::vector<float>(block_size);
	std::vector<float> state_I = std::vector<float>(rf_h.size()-1);
	std::vector<float> state_Q = std::vector<float>(rf_h.size()-1);
	std::vector<float> fm_demod = std::vector<float>(block_size/rf_decim);
	float prev_I = 0, prev_Q = 0;
	std::vector<float> audio_filt = std::vector<float>(block_size/rf_decim);
	std::vector<short> audio = std::vector<short>((block_size/rf_decim)/audio_decim);
	std::vector<float> state_audio = std::vector<float>(audio_h.size()-1);
	std::vector<float>* IQ[] = {&I, &Q};
	std::string sample_string;
	uint8_t byte;

	while(true){
		
		std::cin.read(reinterpret_cast<char*>(IQ_buf.data()), 2*block_size);

		while(sample_num < 2*block_size){
			iq_sample = ((float)IQ_buf[sample_num]-128.0)/128.0;
			IQ_index = sample_num & 0x01;
			(*IQ[IQ_index])[sample_num >> 1] = iq_sample;
			sample_num += 1;
		}
		sample_num = 0;
		
		convolveFIR(I_ds, I, rf_h, state_I, rf_decim);
		/*
		for (int i = 0; i < block_size; i+=rf_decim){
			I_ds[i/rf_decim] = filt_IQ[i];
		}
		*/
		convolveFIR(Q_ds, Q, rf_h, state_Q, rf_decim);
		/*
		for (int i = 0; i < block_size; i+=rf_decim){
			Q_ds[i/rf_decim] = filt_IQ[i];
		}
		*/
		
		fmDemodNoArctan(I_ds, Q_ds, prev_I, prev_Q, fm_demod);
		
		convolveFIR(audio_filt, fm_demod, audio_h, state_audio, audio_decim);
		
		for (int i = 0; i < block_size/rf_decim; i+=audio_decim){
			audio[i/audio_decim] = (short int)(16384*audio_filt[i]);
			//std::cerr << audio[i/audio_decim] << std::endl;
		}
	
		fwrite(&audio[0], sizeof(short int), audio.size(), stdout); 
		
		//for (uint i = 0; i < audio_filt.size(); i+=audio_decim){
			//std::cout << (signed short int)(16384*audio_filt[i]);
			//std::cerr << (signed short int)(16384*audio_filt[i]) << std::endl;
			
			//if (i == 10*audio_decim){ exit(1);
			//}
		//}
		//std::cerr << block_count << "\n";
		//block_count += 1;
		
	}
}

void mono_mode2(){
	
	std::vector<float> audio_h;
	std::vector<float> rf_h;
	
	int rf_Fs = 2.4e6;
	int rf_Fc = 100e3;
	unsigned short int rf_taps = 101;
	int rf_decim = 10;
	int audio_decim = 5;
	int audio_Fs = 240e3;
	int audio_Fc = 16e3;

	int block_size = 1024 * rf_decim * audio_decim;
	int block_count = 0;
	
	impulseResponseLPF(rf_Fs, rf_Fc, rf_taps, rf_h);
	impulseResponseLPF(audio_Fs, audio_Fc, rf_taps, audio_h);
	
	float iq_sample;
	int sample_num = 0;
	int IQ_index;
	
	uint8_t x;
		
	std::vector<uint8_t> IQ_buf = std::vector<uint8_t>(2*block_size);
	std::vector<float> I = std::vector<float>(block_size);
	std::vector<float> Q = std::vector<float>(block_size);
	std::vector<float> I_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> Q_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> filt_IQ = std::vector<float>(block_size);
	std::vector<float> state_I = std::vector<float>(rf_h.size()-1);
	std::vector<float> state_Q = std::vector<float>(rf_h.size()-1);
	std::vector<float> fm_demod = std::vector<float>(block_size/rf_decim);
	float prev_I = 0, prev_Q = 0;
	std::vector<float> audio_filt = std::vector<float>(block_size/rf_decim);
	std::vector<short> audio = std::vector<short>((block_size/rf_decim)/audio_decim);
	std::vector<float> state_audio = std::vector<float>(audio_h.size()-1);
	std::vector<float>* IQ[] = {&I, &Q};
	std::string sample_string;
	uint8_t byte;
	
	while(true){
	
		std::cin.read(reinterpret_cast<char*>(IQ_buf.data()), 2*block_size);

		while(sample_num < 2*block_size){
			iq_sample = ((float)IQ_buf[sample_num]-128.0)/128.0;
			IQ_index = sample_num & 0x01;
			(*IQ[IQ_index])[sample_num >> 1] = iq_sample;
			sample_num += 1;
		}
		sample_num = 0;

		convolveFIR(filt_IQ, I, rf_h, state_I, rf_decim);
		for (int i = 0; i < block_size; i+=rf_decim){
			I_ds[i/rf_decim] = filt_IQ[i];
		}
		convolveFIR(filt_IQ, Q, rf_h, state_Q, rf_decim);
		for (int i = 0; i < block_size; i+=rf_decim){
			Q_ds[i/rf_decim] = filt_IQ[i];
		}
		
		
	}
	
}

void mono_mode3(){
	std::cerr << "TODO! - To be implemented in future renditions.\n";
}
