#include "stereo.h"
#include "filter.h"
#include "args.h"
#include "threadsafequeue.h"
#include "pll.h"
#include "logfunc.h"
#include "fourier.h"
#include "dy4.h"

void stereo(args* p){
	
	int block_size = (1470 * p->audio_decim)/p->audio_upsample;
	
	std::vector<float> audio_h;
	std::vector<float> mono_delay_h;
	std::vector<float> carrier = std::vector<float>(block_size+1, 0.0);
	std::vector<float> pilot_h;
	std::vector<float> stereo_h;
	std::vector<float> carrier_h;
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
	short sample;
	
	std::vector<float>* fm_demod;

	stereo_dc.clear(); stereo_dc.resize(block_size, 0.0);
	carrier.resize(block_size+1, 0.0);
	mono_delay.resize(block_size, 0.0);
	carrier[carrier.size()-1] = 1.0;
	stereo.clear(); stereo.resize(2*((block_size*p->audio_upsample)/p->audio_decim), 0.0);
	mono_delay_state.resize(p->rf_taps-1, 0.0), stereo_state.resize(p->rf_taps-1, 0.0);
	mono_state.resize(p->rf_taps-1, 0.0), extracted_stereo_band_state.resize(p->rf_taps-1, 0.0);
	extracted_pilot_state.resize(p->rf_taps-1, 0.0);

    pllblock_args block_args;
	block_args.feedbackI = 1.0;
    block_args.feedbackQ = 0.0;
    block_args.integrator = 0.0;
    block_args.phaseEst = 0.0;
    block_args.trigOffset = 0.0;
    block_args.lastCarrier = 1.0;
    
	float fb_pilot[] = { 18.5e3, 19.5e3 };
	float fb_carrier[] = { 37.5e3, 38.5e3};
	float fb_stereo[] = { 22e3, 54e3 };
	
	impulseResponseAPF(1, p->rf_taps, mono_delay_h);
	impulseResponseLPF(p->if_Fs*p->audio_upsample, p->audio_Fc, p->rf_taps*p->audio_upsample, audio_h, p->audio_upsample);
	impulseResponseBPF(p->rf_Fs/p->rf_decim, fb_pilot, p->rf_taps, pilot_h);
	impulseResponseBPF(p->rf_Fs/p->rf_decim, fb_carrier, p->rf_taps, carrier_h);
	impulseResponseBPF(p->rf_Fs/p->rf_decim, fb_stereo, p->rf_taps, stereo_h);
	
	while(true){
		// Retrieve demodulated FM data from the front end.
		p->queue.wait_and_pop(fm_demod, 0);
		
		// 19 KHz pilot tone extraction.
		convolveFIR(extracted_pilot, *fm_demod, pilot_h, extracted_pilot_state, 1);
		
		// Locking a 38 KHz tone to 19 KHz pilot using PLL.
		fmpll(extracted_pilot, 19e3, p->rf_Fs/p->rf_decim, carrier, block_args, 2.0, 0, 0.01);
		
		// Stereo channel extraction.
		convolveFIR(extracted_stereo_band, *fm_demod, stereo_h, extracted_stereo_band_state, 1);
		
		// Perform stereo downconversion to baseband by mixing with carrier DSB-SC demodulation.
		for (unsigned int i = 0; i < extracted_stereo_band.size(); i++){
			stereo_dc[i] = 2.0*extracted_stereo_band[i]*carrier[i];
		}
		
		// Delay block (convolution with an impulse shifted in time)
		convolveFIR(mono_delay, *fm_demod, mono_delay_h, mono_delay_state, 1);
		
		// Indicate to RF frontend audio thread is prepared to recieve more demodulated FM data.
		p->queue.prepare(0);

		// Mono channel extraction.
		convolveFIR(mono_filt, mono_delay, audio_h, mono_state, p->audio_upsample, p->audio_decim);
		
		// Stereo channel extraction.
		convolveFIR(stereo_filt, stereo_dc, audio_h, stereo_state, p->audio_upsample, p->audio_decim);
		
		// Interleaving left and right samples into stereo vector + scaling to int16 for aplay.
		while (stereo_sample < 2*(block_size*p->audio_upsample)/p->audio_decim){
			right_sample = static_cast<short int>(16384*(mono_filt[stereo_sample >> 1] - stereo_filt[stereo_sample >> 1]));
			left_sample = static_cast<short int>(16384*(mono_filt[stereo_sample >> 1] + stereo_filt[stereo_sample >> 1]));
			sample = ((left_sample) & ((stereo_sample & 0x01) - 1)) | ((right_sample) & ~((stereo_sample & 0x01) - 1));
			
			stereo[stereo_sample] = sample;
			stereo_sample++;
		}
		stereo_sample = 0;
	
		// Writing to standard out UNIX pipe for aplay.
		fwrite(&stereo[0], sizeof(short int), stereo.size(), stdout);
		
		block_count++;
	}
}
