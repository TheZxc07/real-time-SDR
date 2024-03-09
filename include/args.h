#ifndef ARGS_H
#define ARGS_H

#include "threadsafequeue.h"

struct args {
	ThreadSafeQueue<std::vector<float>*> &queue;
	int rf_Fs;
	int rf_Fc;
	unsigned short int rf_taps;
	int rf_decim;
	int audio_decim;
	int audio_Fs;
	int audio_Fc;
};

#endif // ARGS_H
