#include "pll.h"
#include "dy4.h"

void fmpll(const std::vector<float> &pllIn, float freq, float Fs, std::vector<float> &pllOut, float ncoScale = 1.0, float phaseAdjust = 0.0, float normBandwidth = 0.01){
    float Cp = 2.666;
    float Ci = 3.555;

    float Kp = normBandwidth*Cp;
    float Ki = normBandwidth*normBandwidth*Ci;

    pllOut.clear(); pllOut.resize(pllIn.size()+1);

	float integrator = 0.0;
	float phaseEst = 0.0;
	float feedbackI = 1.0;
	float feedbackQ = 0.0;
	pllOut[0] = 1.0;
	float trigOffset = 0;
    float trigArg;

    float errorI, errorQ, errorD;

    for (int i = 0; i < pllIn.size(); i++){

        errorI = pllIn[i]*(feedbackI);
        errorQ = pllIn[i]*(-feedbackQ);

        errorD = atan2(errorQ, errorI);

        integrator = integrator + Ki*errorD;
        phaseEst = phaseEst + Kp*errorD + integrator;
        
        trigOffset += 1.0;
        trigArg = 2*PI*(freq/Fs)*(trigOffset)+phaseEst;
		feedbackI = cos(trigArg);
		feedbackQ = sin(trigArg);

		pllOut[i+1] = cos(trigArg*ncoScale + phaseAdjust);
    }
}