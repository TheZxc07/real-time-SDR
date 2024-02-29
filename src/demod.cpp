#include "demod.h"

void fmDemodNoArctan(const std::vector<float> &I, const std::vector<float> &Q, float prev_I, float prev_Q, std::vector<float> &fm_demod, int decimation_factor)){
	
	fm_demod.clear(); fm_demod.resize(I.size());
	uint i;
	
	fm_demod[0] = (I[0]*(Q[0]-prev_Q)-Q[0]*(I[0]-prev_I))/(pow(I[0], 2.0) + pow(Q[0],2.0));
	for (i = decimation_factor; i < I.size(); i += decimation_factor){ //updated rate at which we itterate through I and Q
		if ((I[i] == 0) & (Q[i] == 0)){
			fm_demod[i] = 0;
		} else{
			fm_demod[i] = (I[i]*(Q[i]-Q[i-1])-Q[i]*(I[i]-I[i-1]))/(pow(I[i], 2.0) + pow(Q[i],2.0));
		}
	}
	
	prev_I = I[i-1];
	prev_Q = Q[i-1];
	
}
