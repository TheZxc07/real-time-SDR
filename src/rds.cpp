#include "rds.h"
#include "args.h"
#include "filter.h"
#include "pll.h"
#include "rds_utilities.h"

void rds(args* p){

    std::vector<float> rds_h;
    std::vector<float> rds_delay_h;
    std::vector<float> rds_baseband_h;
    std::vector<float> pilot_h;
    std::vector<float> rrc_h;
    std::vector<float>* fm_demod;

    int audio_decim = p->audio_decim;
	int audio_upsample = p->audio_upsample;
	int if_Fs = p->if_Fs;
    unsigned short int rf_taps = p->rf_taps;
    int block_size = (1470 * audio_decim)/audio_upsample;
    int sps = p->symbol_Fs;
    
    int block_count = 0;
    int half_symbol = 0;
    int start = 0;
    int last_bit = 0;

    std::vector<float> rds_band;
    std::vector<float> rds_band_squared; rds_band_squared.resize(block_size, 0.0);
    std::vector<float> rds_band_state; rds_band_state.clear(); rds_band_state.resize(rf_taps-1, 0.0);
    std::vector<float> gen_pilot;
    std::vector<float> gen_pilot_state; gen_pilot_state.clear(); gen_pilot_state.resize(rf_taps-1, 0.0);
    std::vector<float> IPLL; IPLL.resize(block_size+1, 0.0); IPLL[0] = IPLL[IPLL.size()-1];
    std::vector<float> rds_band_delay;
    std::vector<float> rds_band_delay_state; rds_band_delay_state.clear(); rds_band_delay_state.resize(rf_taps-1,0.0);
    std::vector<float> rds_dc; rds_dc.resize(block_size, 0.0);
    std::vector<float> rds_filt; 
    std::vector<float> rds_filt_state; rds_filt_state.clear(); rds_filt_state.resize(rf_taps-1,0.0);
    std::vector<float> rds_clean;
    std::vector<float> rds_clean_state; rds_clean_state.clear(); rds_clean_state.resize(rf_taps-1, 0.0);
    std::vector<int> symbols; symbols.clear(); symbols.resize(2836/39, 0.0);
    std::vector<int> bits;
    std::vector<int> decoded_bits;

    pllblock_args block_args;
	block_args.feedbackI = 1.0;
    block_args.feedbackQ = 0.0;
    block_args.integrator = 0.0;
    block_args.phaseEst = 0.0;
    block_args.trigOffset = 0.0;

    float fb_rds[] = { 54e3, 60e3 };
    float fb_rds_squared[] = { 113.5e3, 114.5e3 };

    impulseResponseLPF(if_Fs*247, 3e3, rf_taps*247, rds_baseband_h, 247);
    impulseResponseBPF(if_Fs, fb_rds, rf_taps, rds_h);
    impulseResponseBPF(if_Fs, fb_rds_squared, rf_taps, pilot_h);
    impulseResponseAPF(1, rf_taps, rds_delay_h);
    impulseResponseRRC(2375*sps, rf_taps, rrc_h);

    int sample_offset = 0;

    while(true){
        p->queue.wait_and_pop(fm_demod);
        
        // RDS band extraction through BPF
        convolveFIR(rds_band, *fm_demod, rds_h, rds_band_state, 1);

        p->queue.prepare();
        
        // Squaring Non-Linearity to generate pilot at 114 KHz
        for (int i = 0; i < block_size; i++){
            rds_band_squared[i] = rds_band[i]*rds_band[i];
        }
      
        // 114 KHz pilot extraction through BPF
        convolveFIR(gen_pilot, rds_band_squared, pilot_h, gen_pilot_state, 1);
        
        // Generate a 57 KHz tone from 114 KHz pilot
        fmpll(gen_pilot, 114e3, if_Fs, IPLL, block_args, 0.5, 0, 0.001);

        // Delay the RDS band to phase align with the carrier
        convolveFIR(rds_band_delay, rds_band, rds_delay_h, rds_band_delay_state, 1);

        // Mixing to downconvert RDS band to baseband
        for (int i = 0; i < block_size; i++){
            rds_dc[i] = 2.0*rds_band_delay[i]*IPLL[i];
        }

        // Extract RDS band through LPF
        convolveFIR(rds_filt, rds_dc, rds_baseband_h, rds_filt_state, 247, 640);
	
        // Pass RDS band through RRC filter to reduce ISI
        convolveFIR(rds_clean, rds_filt, rrc_h, rds_clean_state, 1);

        // Perform clock and data recovery to produce sample offset
        sample_offset = cdr(sps, rds_clean);

        // Take every sps-th element starting at offset from the clean RDS signal to recover symbols
        for (int i = 0; i < symbols.size(); i++){
            symbols[i] = rds_clean[sample_offset + i*sps] > 0;
        }

        // Perform Manchester decoding to extract bits from symbols
        manchester_decode(bits, symbols, block_count, half_symbol, start);

        // Perform differential decoding to recover bitstream
        differential_decode(decoded_bits, bits, last_bit, block_count);

		block_count++;
    }

}
