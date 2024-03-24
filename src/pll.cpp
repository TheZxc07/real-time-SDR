#include "pll.h"
#include "dy4.h"
#include <iomanip>
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
    //pllOut.resize(pllIn.size() + 1);
    pllOut[0] = pllOut[pllOut.size()-1];
    //std::cerr << "Integrator: " << block.integrator << std::endl;
    //std::cerr << "PhaseEst: " << block.phaseEst << std::endl;
    //std::cerr << "============Beginning============" << std::endl;
    //std::cerr << "FeedbackI: " << block.feedbackI << std::endl;
    //std::cerr << "FeedbackQ: " << block.feedbackQ << std::endl;
    //std::cerr << "=================================" << std::endl;
    //std::cerr << "LastCarrier: " << block.lastCarrier << std::endl;
    //std::cerr << "TrigOffset: " << std::fixed << block.trigOffset << std::endl;
    //std::cerr << "NormBandwidth: " << normBandwidth << std::endl;
    //float trigOffset = 0;
    float trigArg;

    float errorI, errorQ, errorD = 0;
    
    float prevD = 0;
    
    //std::cerr << Fs << std::endl;
    for (int i = 0; i < (int)pllIn.size(); i++){

        errorI = pllIn[i]*(block.feedbackI);
        errorQ = pllIn[i]*(-block.feedbackQ);
	//prevD = errorD;
        errorD = atan2(errorQ, errorI);
	
        block.integrator = block.integrator + Ki*errorD;
        block.phaseEst = block.phaseEst + Kp*errorD + block.integrator;
	
        //2*PI*(freq/Fs)*(block.trigOffset)
	
        block.trigOffset += 1.0;
	trigArg = 2*PI*(freq/Fs)*(block.trigOffset) + block.phaseEst;
	
	block.feedbackI = cos(trigArg);
	block.feedbackQ = sin(trigArg);

	pllOut[i+1] = cos(trigArg*ncoScale + phaseAdjust);
    }
    //std::cerr << "===============End===============" << std::endl;
    //std::cerr << "FeedbackI: " << block.feedbackI << std::endl;
    //std::cerr << "FeedbackQ: " << block.feedbackQ << std::endl;
    //std::cerr << "=================================" << std::endl;
    block.lastCarrier = pllOut[pllOut.size()-1];
    //pllOut.erase(pllOut.end() - 1, pllOut.end());
    //cont++;
}
