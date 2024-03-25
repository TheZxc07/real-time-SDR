#ifndef ARGS_H
#define ARGS_H

#include "threadsafequeue.h"

struct args {
	ThreadSafeQueue<std::vector<float>*> &queue;
	int rf_Fs;
	int rf_Fc;
	unsigned short int rf_taps;
	int rf_decim;
	float audio_decim;
	float audio_upsample;
	int if_Fs;
	int audio_Fc;
	int audio_Fs;
	int symbol_Fs;
};

#endif // ARGS_H
