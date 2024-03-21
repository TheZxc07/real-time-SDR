#include "stereo.h"
#include "filter.h"
#include "args.h"
#include "threadsafequeue.h"
#include "pll.h"
#include "logfunc.h"

void stereo_mode0(args* p){
	
	int block_size = 1024 * p->audio_decim;
	
	std::vector<float> audio_h;
	std::vector<float> mono_delay_h;
	std::vector<float> carrier = std::vector<float>(block_size+1, 0.0);
	std::vector<float> pilot_h;
	std::vector<float> stereo_h;
	std::vector<float> extracted_pilot(block_size, 0.0);
	std::vector<float> extracted_pilot_state(p->rf_taps-1, 0.0);
	std::vector<float> extracted_stereo_band;
	std::vector<float> extracted_stereo_band_state = std::vector<float>(p->rf_taps-1);
	std::vector<float> stereo_dc;
	std::vector<float> mono;
	std::vector<float> mono_state = std::vector<float>(p->rf_taps-1);
	std::vector<float> mono_filt;
	std::vector<float> mono_delay = std::vector<float>(block_size);
	std::vector<float> mono_delay_state = std::vector<float>(p->rf_taps-1);
	std::vector<float> stereo_filt;
	std::vector<float> stereo_state = std::vector<float>(p->rf_taps-1);
	std::vector<short> stereo;
	
	int block_count = 0;
	int stereo_sample = 0;
	short left_sample;
	short right_sample;
	//float left;
	//float right;
	short sample;
	
	std::vector<float>* fm_demod;

	stereo_dc.clear(); stereo_dc.resize(block_size, 0.0);
	carrier.resize(block_size+1, 0.0);
	mono_delay.resize(block_size, 0.0);
	carrier[carrier.size()-1] = 1.0;
	stereo.clear(); stereo.resize(2*(block_size/p->audio_decim), 0.0);
	mono_delay_state.resize(p->rf_taps-1, 0.0), stereo_state.resize(p->rf_taps-1, 0.0);
	mono_state.resize(p->rf_taps-1, 0.0), extracted_stereo_band_state.resize(p->rf_taps-1, 0.0);
	extracted_pilot_state.resize(p->rf_taps-1, 0.0);
		
	float feedbackI = 1.0;
    float feedbackQ = 0.0;
    float integrator = 0.0;
    float phaseEst = 0.0;
    float trigOffset = 0;
    
    pllblock_args block_args = {feedbackI, feedbackQ, integrator, phaseEst, trigOffset};
    
	float fb_pilot[] = { 18.5e3, 19.5e3 };
	float fb_stereo[] = { 22e3, 54e3 };
	std::vector<float> x = std::vector<float>(carrier.size());
	
	impulseResponseAPF(1, p->rf_taps, mono_delay_h);
	impulseResponseLPF(p->rf_Fs/p->rf_decim, p->audio_Fc, p->rf_taps, audio_h);
	impulseResponseBPF(p->rf_Fs/p->rf_decim, fb_pilot, p->rf_taps, pilot_h);
	impulseResponseBPF(p->rf_Fs/p->rf_decim, fb_stereo, p->rf_taps, stereo_h);
	std::string file = "lockedpilot";
	std::string file1 = "extractedpilot";
	
	std::vector<float> pllCheck;
	std::vector<float> ncoCheck;
	std::vector<float> stereoCheck;
	while(true){
		while(!(p->queue.empty())){
			//std::cerr << "Processing block: " << block_count << "\n";
			p->queue.wait_and_pop(fm_demod);
			
			// 19 KHz pilot tone extraction
			convolveFIR(extracted_pilot, *fm_demod, pilot_h, extracted_pilot_state, 1); 
			
			// Locking a 38 KHz tone to 19 KHz pilot using PLL
			fmpll(extracted_pilot, 19e3, p->rf_Fs/p->rf_decim, carrier, block_args, 2.0);
			
			
			/*
			if(block_count <= 10)
			{
				pllCheck.insert(pllCheck.end(), extracted_pilot.begin(), extracted_pilot.end());
				ncoCheck.insert(ncoCheck.end(), carrier.begin(), carrier.end());
				if(block_count == 10)
				{
					std::vector<float> index;
					genIndexVector(index, ncoCheck.size());
					logVector("PLL", index, pllCheck);
					logVector("NCO", index, ncoCheck);
					exit(1);
				}
			}
			*/
			
			
			// Stereo channel extraction
			convolveFIR(extracted_stereo_band, *fm_demod, stereo_h, extracted_stereo_band_state, 1);

			// Perform stereo downconversion to baseband by mixing with carrier DSB-SC demodulation.
			for (unsigned int i = 0; i < carrier.size(); i++){
				stereo_dc[i] = 2.0*extracted_stereo_band[i]*carrier[i];
				//std::cerr << stereo_dc[i] << std::endl;
			}
			
			// Delay block (convolution with an impulse shifted in time)
			convolveFIR(mono_delay, *fm_demod, mono_delay_h, mono_delay_state, 1);
			/*
			for (int i = 50; i < fm_demod->size(); i++){
				mono_delay[i] = (*fm_demod)[i-50];
			}
			*/
			
			/*
			for (int i = 0; i < (*fm_demod).size() + ((p->rf_taps-1)/2); i++){
				if (i < ((p->rf_taps-1)/2)) {
					mono_delay[i] = mono_delay_state[i];
				}else if(i > (*fm_demod).size()){
					mono_delay_state[i - (*fm_demod).size()] = (*fm_demod)[i];
				}else {
					mono_delay[i] = (*fm_demod)[i - ((p->rf_taps-1)/2)];
				}
			}
			*/
			
			// Mono channel extraction
			convolveFIR(mono_filt, mono_delay, audio_h, mono_state, p->audio_decim);
			// Stereo channel extraction
			convolveFIR(stereo_filt, stereo_dc, audio_h, stereo_state, p->audio_decim);
			
			// Interleaving left and right samples into stereo vector
			while (stereo_sample < 2*block_size/p->audio_decim){
				right_sample = (short int)(16384*(mono_filt[stereo_sample >> 1] - stereo_filt[stereo_sample >> 1]));
				left_sample = (short int)(16384*(mono_filt[stereo_sample >> 1] + stereo_filt[stereo_sample >> 1]));
				sample = ((left_sample >> 1) & ((stereo_sample & 0x01) - 1)) | ((right_sample >> 1) & ~((stereo_sample & 0x01) - 1));
				
				stereo[stereo_sample] = sample;
				stereo_sample++;
			}
			stereo_sample = 0;
			
			fwrite(&stereo[0], sizeof(short int), stereo.size(), stdout);
	
			delete fm_demod;
			block_count++;
		}
		
	}
	 
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
