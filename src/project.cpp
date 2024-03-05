/*
Comp Eng 3DY4 (Computer Systems Integration Project)

Copyright by Nicola Nicolici
Department of Electrical and Computer Engineering
McMaster University
Ontario, Canada
*/

#include "dy4.h"
#include "filter.h"
#include "fourier.h"
#include "genfunc.h"
#include "iofunc.h"
#include "logfunc.h"
#include "mono.h"
#include "stereo.h"
#include "rds.h"
#include "utilities.h"
#include <map>

int main(int argc, char* argv[])
{
	
	if (argc < 3){
		mono_mode0();
	} 
	
	void (*funcs[13])(void) = {shutdown, mono_mode0, mono_mode1, mono_mode2, mono_mode3,
				   stereo_mode0, stereo_mode1, stereo_mode2, stereo_mode3,
				   rds_mode0, rds_mode1, rds_mode2, rds_mode3};
	bandtype type;
	switch((int)(*argv[2])){
		case 109:
			type = M;
			break;
		case 115:
			type = S;
			break;
		case 114:
			type = R;
			break;
		default:
			shutdown();
			break;
	}
	
	int mode = (int)(*argv[1])-'0';
	int func_index = type + mode + 1;
	int flag = (((mode >> 2) - 1) >> 31); 
	funcs[func_index & flag](); 
	
}
