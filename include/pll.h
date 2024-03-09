#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

#ifndef PLLBLOCK_ARGS_H
#define PLLBLOCK_ARGS_H


struct pllblock_args {
    float &feedbackI;
    float &feedbackQ;
    float &integrator;
    float &phaseEst;
    float &trigOffset;
};


void fmpll(const std::vector<float> &, float, float, std::vector<float> &, pllblock_args &, float = 1.0, float = 0.0, float = 0.01);


#endif // PLLBLOCK_ARGS_H
