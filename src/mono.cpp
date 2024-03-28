
#include "mono.h"
#include "filter.h"
#include "args.h"
#include "threadsafequeue.h"

void mono(args* p){
	
	std::vector<float> audio_h;

	unsigned short int rf_taps = p->rf_taps;
	//int rf_decim = p->rf_decim;
	int audio_decim = p->audio_decim;
	int audio_upsample = p->audio_upsample;
	int if_Fs = p->if_Fs;
	int audio_Fc = p->audio_Fc;

	int block_size = (1470 * audio_decim)/audio_upsample;
	int block_count = 0;
	
	impulseResponseLPF(if_Fs*audio_upsample, audio_Fc, rf_taps*audio_upsample, audio_h, audio_upsample);
	
	std::vector<float>* fm_demod;
	std::vector<float> audio_filt = std::vector<float>(block_size);
	std::vector<short> audio = std::vector<short>(block_size*audio_upsample/audio_decim);
	std::vector<float> state_audio = std::vector<float>(audio_h.size()-1);

	while(true){
		// Retrieve demodulated FM data from the front end.
		p->queue.wait_and_pop(fm_demod, 0);

		// Mono subchannel extraction.
		convolveFIR(audio_filt, *fm_demod, audio_h, state_audio, audio_upsample, audio_decim);
		
		// Indicate to RF frontend, audio thread is prepared to recieve more demodulated FM data.
		p->queue.prepare(0);
		
		// Scaling to int16 for aplay.
		for (unsigned int i = 0; i < audio.size(); i++){
			audio[i] = static_cast<short int>(16384*audio_filt[i]);
		}
	
		// Writing to standard out UNIX pipe for aplay.
		fwrite(&audio[0], sizeof(short int), audio.size(), stdout); 
		
		block_count++;
			
	}
}
