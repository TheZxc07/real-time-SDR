#include "rffrontend.h"
#include "filter.h"
#include "threadsafequeue.h"
#include "demod.h"
#include "iofunc.h"
#include "logfunc.h"
#include "fourier.h"

void RF_frontend(args* p){
	
	std::vector<float> rf_h;
	
	int rf_Fs = p->rf_Fs;
	int rf_Fc = p->rf_Fc;
	unsigned short int rf_taps = p->rf_taps;
	int rf_decim = p->rf_decim;
	int audio_decim = p->audio_decim;
	int audio_upsample = p->audio_upsample;
	//int audio_Fs = 240e3;  //Unused 
	//int audio_Fc = 16e3;	//Unused 
	int block_size = (1470 * rf_decim * audio_decim)/audio_upsample;
	int block_count = 0;
	
	impulseResponseLPF(rf_Fs, rf_Fc, rf_taps, rf_h);
	
	float iq_sample;
	int sample_num = 0;
	int IQ_index;
		
	std::vector<uint8_t> IQ_buf = std::vector<uint8_t>(2*block_size);
	std::vector<float> I = std::vector<float>(block_size);
	std::vector<float> Q = std::vector<float>(block_size);
	std::vector<float> I_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> Q_ds = std::vector<float>(block_size/rf_decim);
	std::vector<float> filt_IQ = std::vector<float>(block_size);
	std::vector<float> state_I = std::vector<float>(rf_h.size()-1);
	std::vector<float> state_Q = std::vector<float>(rf_h.size()-1);
	state_I.clear(); state_I.resize(rf_h.size()-1, 0.0);
	state_Q.clear(); state_Q.resize(rf_h.size()-1, 0.0);
	std::vector<float>* fm_demod;// = std::vector<float>(block_size/rf_decim);
	//std::vector<float>* fm_demod_ptr; 
	float prev_I = 0, prev_Q = 0;
	std::vector<float>* IQ[] = {&I, &Q};
	
	while(true){
		
		// Read from standard input buffer raw IQ data <- RF dongle.
		std::cin.read(reinterpret_cast<char*>(&IQ_buf[0]), 2*block_size*sizeof(char));
		
		if(std::cin.eof()){
			exit(1);
		}
		
		// Initialize new block of FM demodulated data.
		fm_demod = new std::vector<float>(block_size/rf_decim);

		// Seperate I and Q samples into individual vectors.
		while(sample_num < 2*block_size){
			iq_sample = float(((unsigned char)IQ_buf[sample_num]-128.0)/128.0);
			IQ_index = sample_num & 0x01;
			(*IQ[IQ_index])[sample_num >> 1] = iq_sample;
			sample_num += 1;
		}
		sample_num = 0;
		
		// Extract 100 KHz FM band channel through LPF.
		convolveFIR(I_ds, I, rf_h, state_I, rf_decim);
		convolveFIR(Q_ds, Q, rf_h, state_Q, rf_decim);
		
		// Demodulate FM data.
		fmDemodNoArctan(I_ds, Q_ds, prev_I, prev_Q, *fm_demod);
		
		// Push block of FM data onto sync queue.
		p->queue.push(fm_demod);
		block_count++;
	}
}
