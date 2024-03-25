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
#include "rffrontend.h"
#include "threadsafequeue.h"
#include "args.h"
#include <thread>
#include <functional>

int main(int argc, char* argv[])
{
	ThreadSafeQueue<std::vector<float>*> queue;
	void (*audio_func) (args*); 

	args func_args = {
		queue,
		2400000,
		100000,
		101,
		10,
		5,
		1,
		240000,
		16000,
		48000,
		39
	};
	
	if (argc < 3){
		audio_func = &mono;
	} else {
	
		/*
		void (*funcs[13])(void) = {shutdown, mono_mode0, mono_mode1, mono_mode2, mono_mode3,
					stereo_mode0, stereo_mode1, stereo_mode2, stereo_mode3,
					rds_mode0, rds_mode1, rds_mode2, rds_mode3};
		*/			   
		//std::thread rf_frontend_thread(&RF_frontend, &func_args);
		//std::thread audio_thread(&stereo_mode0, &func_args);
		
		//int rf_Fs;
		//int rf_decim; 
		//audio_Fs
		//audio_Fc
		//int audio_decim to change 

		//bandtype type;  UNUSED VAR


		switch((int)(*argv[1])){
			case 48:
				// up is default to 1
				func_args.rf_Fs= 2.4e6;
				func_args.rf_decim = 10;
				func_args.audio_decim=5;
				func_args.if_Fs=240e3;
				//func_args.down = 5;
				break;
			case 49:
				func_args.rf_Fs= 1.44e6;
				func_args.rf_decim = 4;
				func_args.audio_decim=9;
				func_args.if_Fs=360e3;
				//func_args.down = 9;
				break;
			case 50:
				func_args.rf_Fs= 2.4e6;
				func_args.rf_decim = 10;
				func_args.audio_decim=800;
				func_args.if_Fs=240e3;
				func_args.audio_upsample=147;
				func_args.symbol_Fs=20;
				//func_args.down=800;
				// have to add upsample and downsample
				break;
			
			case 51:
				func_args.rf_Fs=  1.152e6;
				func_args.rf_decim = 3;
				func_args.audio_decim=1280;
				func_args.if_Fs=384e3;
				func_args.audio_upsample=147;
				func_args.symbol_Fs=20;
				//func_args.down=1280;
				// have to add upsample and downsample
				break;
			
			default:
				shutdown();
				break;
		}

		//std::thread rf_frontend_thread(&RF_frontend, &func_args);
		switch((int)(*argv[2])){
			// Type M
			case 109: 
				audio_func = &mono;
				//std::cerr << "!!" << std::endl;
				//std::thread audio_thread(&mono_mode0, &func_args);
				break;
			// Type S
			case 115:
				audio_func = &stereo_mode0;
				//std::thread audio_thread(&stereo_mode0, &func_args);
				break;
			// Type R
			case 114:
				audio_func = &rds_mode0;
				audio_func = &stereo_mode0;
				//std::thread audio_thread(&rds_mode0, &func_args);
				break;
			default:
				shutdown();
			break;
		}
	}
	std::thread audio_thread(audio_func, &func_args);
	std::thread rf_frontend_thread(&RF_frontend, &func_args);
	audio_thread.join();
	rf_frontend_thread.join();
	
	//int mode = (int)(*argv[1])-'0';
	//int func_index = type + mode + 1;
	//int flag = (((mode >> 2) - 1) >> 31); 
	//funcs[func_index & flag](); 
	
}
