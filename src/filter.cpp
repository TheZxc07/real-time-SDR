/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#include "dy4.h"
#include "filter.h"

// function to compute the impulse response "h" based on the sinc function
void impulseResponseLPF(float Fs, float Fc, unsigned short int num_taps, std::vector<float> &h)
{
	// bring your own functionality
	h.clear(); h.resize(num_taps, 0.0);

	float normalized_cutoff = Fc/(Fs/2.0);

	for (int i = 0; i < num_taps; i++){

		if (i == (num_taps-1.0)/2.0){
			h[i] = normalized_cutoff;
		} else {
			h[i] = normalized_cutoff*sin(PI*normalized_cutoff*(i-(num_taps-1.0)/2.0))/(PI*normalized_cutoff*(i-(num_taps-1.0)/2.0));
		}
		h[i] = h[i]*sin(i*PI/((float)num_taps))*sin(i*PI/((float)num_taps));
	}
}

void impulseResponseBPF(float Fs, float[] Fb, unsigned short int num_taps, std::vector<float> &h)
{
	h.clear(); h.resize(num_taps, 0.0);
	
	float normalized_center = ((Fb[1] + Fb[0])/2)/(Fs/2);
	float normalized_pass = ((Fb[1] - Fb[0])/2)/(Fs/2);
	
	for (int i = 0; i < num_taps; i++){
		if (i == (num_taps-1.0)/2.0){
			h[i] = normalized_pass;
		} else {
			h[i] = normalized_pass*((sin(PI*(normalized_pass/2)*(i-(num_taps-1)/2)))/(PI*(normalized_pass/2)*(i-(num_taps-1)/2)));
		}
		h[i] = h[i]*cos(i*PI*normalized_center);
		h[i] = h[i]*sin(i*PI/((float)num_taps))*sin(i*PI/((float)num_taps));
	}
}


// function to compute the filtered output "y" by doing the convolution
// of the input data "x" with the impulse response "h"
void convolveFIR(std::vector<float> &y, const std::vector<float> &x, const std::vector<float> &h, std::vector<float> &initial_state, const int decimation_factor)
{
	y.clear(); y.resize(x.size()/decimation_factor);
	for (int n = 0; n < x.size(); n+=decimation_factor) {
		for (int k = 0; k < h.size(); k++){
			if (n-k < 0) {
				y[n/decimation_factor] += h[k]*initial_state[n-k+initial_state.size()];
			}else{
				y[n/decimation_factor] += h[k]*x[n-k];
			}
		}
	}

	initial_state = std::vector<float>(x.end()-h.size()+1,x.end());
	
}
