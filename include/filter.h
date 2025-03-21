/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#ifndef DY4_FILTER_H
#define DY4_FILTER_H

// add headers as needed
#include <iostream>
#include <vector>
#include <cmath>

// declaration of a function prototypes
void impulseResponseLPF(float, float, unsigned short int, std::vector<float> &);
void impulseResponseLPF(float, float, unsigned short int, std::vector<float> &, int);
void convolveFIR(std::vector<float> &, const std::vector<float> &, const std::vector<float> &, std::vector<float> &, int);
void impulseResponseBPF(float, float*, unsigned short int, std::vector<float> &);
void impulseResponseAPF(float, unsigned short int, std::vector<float> &);
void impulseResponseRRC(float, unsigned short int, std::vector<float> &);
void convolveFIR(std::vector<float> &, const std::vector<float> &, const std::vector<float> &, std::vector<float> &, int,int);

#endif // DY4_FILTER_H
