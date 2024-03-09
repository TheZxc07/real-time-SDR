#include "pll.h"
#include "dy4.h"
void fmpll(const std::vector<float> &pllIn, float freq, float Fs, std::vector<float> &pllOut, pllblock_args &block, float ncoScale, float phaseAdjust, float normBandwidth){
    float Cp = 2.666;
    float Ci = 3.555;

    float Kp = normBandwidth*Cp;
    float Ki = normBandwidth*normBandwidth*Ci;

    //pllOut.clear(); pllOut.resize(pllIn.size()+1);

    //float integrator = 0.0;
    //float phaseEst = 0.0;
    //float feedbackI = 1.0;
    //float feedbackQ = 0.0;
    //llOut[0];
    //float trigOffset = 0;
    float trigArg;

    float errorI, errorQ, errorD = 0;
    
    float prevD;

    for (int i = 0; i < pllIn.size(); i++){

        errorI = pllIn[i]*(block.feedbackI);
        errorQ = pllIn[i]*(-block.feedbackQ);
	prevD = errorD;
        errorD = atan2(errorQ, errorI);
	
	if (std::isnan(errorD)){
	    errorD = prevD;
	}
	
        block.integrator = block.integrator + Ki*errorD;
        block.phaseEst = block.phaseEst + Kp*errorD + block.integrator;
        
        block.trigOffset += 1.0;
	trigArg = 2*PI*(freq/Fs)*(block.trigOffset)+block.phaseEst;
	block.feedbackI = cos(trigArg);
	block.feedbackQ = sin(trigArg);

	pllOut[i+1] = cos(trigArg*ncoScale + phaseAdjust);
    }
}
