#include "rds_utilities.h"


int cdr(int sps, const std::vector<float> &signal){
    int maxi = 0, maxv = 0, sum = 0;

    for (int i = 0; i < sps; i++){
        for (int k = 0; k < (int)signal.size()/sps; k++){
            sum += abs(signal[k*sps + i]);
        }

        if (sum > maxv){
            maxv = sum;
            maxi = i;
        }

        sum = 0;
    }

    return maxi;
}

int manchester_united(const std::vector<float> &samples){
    int score = 0;

    for (int i = 0; i < (int)samples.size() - 1; i+=2){
        score += (samples[i]*samples[i+1] > 0); 
    }

    return ~(score >> 31) & 0x01;

}

void manchester_decode(std::vector<int> &bits, const std::vector<int> &symbols, int &block_count, int &half_symbol, int &start){
    bits.clear(); //bits.resize(symbols.size()/2 - 1, 0.0);
    int score;

    if (start){
        bits.push_back(half_symbol);
    }

    if (block_count == 0){
        score = 0;
        for (int i = 0; i < (int)symbols.size()-1; i+=2){
            score+=symbols[i]^symbols[i+1];
        }
        for (int j = 1; j < (int)symbols.size()-1; j+=2){
            score-=symbols[j]^symbols[j+1];
        }
        start = score < 0;
    }
    
    //std::cerr << "START: " << start << std::endl;

    for (int i = start; i < (int)symbols.size()-1; i+=2){
        //std::cerr << "i: " << i << " " << "symbol: " << symbols[i] << std::endl;
        
        bits.push_back(symbols[i]);
    }

    if (((symbols.size()-start) & 0x01) == 1){
        half_symbol = symbols[symbols.size()-1];
        start = 1;
    } else{
        start = 0;
    }

}

void differential_decode(std::vector<int> &decoded_bits, const std::vector<int> &bits, int &last_bit, int &block_num){
    
    decoded_bits.clear(); decoded_bits.resize(bits.size(), 0);    
    //std::cerr << bits.size() << std::endl;

    decoded_bits[0] = bits[0];

    if (block_num == 0){
        decoded_bits[0] = bits[0];
    } else {
        decoded_bits[0] = bits[0]^last_bit;
    }

    for (int i = 1; i < (int)bits.size(); i++){
        decoded_bits[i] = bits[i]^bits[i-1];
    }

    last_bit = bits[bits.size()-1];
}

uint64_t calc_syndrome(uint64_t x, uint64_t mlen){
    uint64_t reg = 0;
    uint64_t plen = 10;

    for (int i = mlen; i > 0; i--){
        reg = (reg << 1) | ((x >> (i-1)) & 0x01);
        if (reg & (1 << plen)){
            reg = reg ^ 0x5B9;
        }
    }

    for (int i = plen; i > 0; i--){
        reg = reg << 1;
        if (reg & (1 << plen)){
            reg = reg ^ 0x5B9;
        }
    }

    return reg & ((1 << plen) - 1);
}

void stringify(uint64_t characters, char* combined){
    //char combined[9]; //one char for null termination
    
    for (int i = 0; i < 8; i++ ){
        combined[7-i] = (characters >> (i << 3)) & 0xFF; 
    }
    combined [8] = 0x0;
    
}


const char* PTY_codes[] = {
    "Undefined",
    "News",
    "Information",
    "Sports",
    "Talk",
    "Rock",
    "Classic Rock",
    "Adult Hits",
    "Soft Rock",
    "Top 40",
    "Country",
    "Oldies",
    "Soft",
    "Nostalgia",
    "Jazz",
    "Classical",
    "Rhythm & Blues",
    "Soft Rhythm & Blues",
    "Language",
    "Religious Music",
    "Religious Talk",
    "Personality",
    "Public",
    "College",
    "Spanish Talk",
    "Spanish Music",
    "Hip Hop",
    "Unassigned",
    "Unassigned",
    "Weather",
    "Emergency Test",
    "Emergency"
};

void parse (const uint64_t &bytes, uint64_t &chars, uint64_t &output, bool &first_time){
    uint8_t group_type = (bytes >> 44) &0xF;
    uint8_t AB_flag = (bytes >> 43) & 0x1;
    uint8_t placement = (bytes >> 32) & 0x3;
    uint16_t PI = (bytes >> 48) & 0xFFFF;
    uint8_t PTY_data = (bytes >> 37) & 0x1F;
    
    if (first_time){
        std::cerr << "PI: " << PI << std::endl;
        std::cerr << "PTY: " << PTY_codes[PTY_data] << std::endl;
        first_time = false;
    }
    
    if (group_type == 0){
        chars = (chars << 16) | (bytes & 0xFFFF); //shift up characters, add the new ones from the last block
        //note, for 0A or 0B, the last 16 data bits are unchanged
        if ((placement == 3) && (chars != output)){ //if we've filled 8 characters
            output = chars; //update state
            char str[9];
            stringify(chars, str);
            std::cerr << "Prorgam Service: " << str << std::endl;
        }
    }
}


void error_detection(uint64_t &reg, uint64_t &chars, uint64_t &output, bool &first_time, int &sync, int &prevsync, int &lastseen_offset, int &rds_bit_cont, int &lastseen_offset_cont, int &block_distance, int &block_number, int &block_bit_cont, int &blocks_cont, int &wrong_blocks_cont, int &group_assembly_started, int &group_good_blocks_cont, const std::vector<int> &decoded_bits){
    
    uint64_t reg_syndrome;
    uint64_t syndromes[] = { 383, 14, 303, 663, 748 };
    uint64_t offset_word[] = { 252, 408, 360, 436, 848 };
    int offset_pos[] = { 0, 1, 2, 3, 2};
    
    for (int i = 0; i < (int)decoded_bits.size(); i++){
        reg = (reg << 1) | decoded_bits[i];
        if(!sync){
            reg_syndrome = calc_syndrome(reg, 26);
            //std::cerr << "Reg Syndrome: " << reg_syndrome << "    Reg: " << reg << std::endl;
            for (int j = 0; j < 5; j++){
                if (reg_syndrome == syndromes[j]){
                    if (!prevsync){
                        lastseen_offset = j;
                        lastseen_offset_cont = rds_bit_cont;
                        prevsync = 1;
                    } else{
                        if (offset_pos[lastseen_offset] >= offset_pos[j]){
                            block_distance = offset_pos[j] + 4 - offset_pos[lastseen_offset];
                        } else{
                            block_distance = offset_pos[j] - offset_pos[lastseen_offset];
                        }
                        if ((block_distance*26) != (rds_bit_cont - lastseen_offset_cont)){
                            prevsync = 0;
                        } else{
                            std::cerr << "Sync State Detected" << std::endl;
                            wrong_blocks_cont = 0;
                            blocks_cont = 0;
                            block_bit_cont = 0;
                            block_number = (j+1) & 0x03;
                            group_assembly_started = 0;
                            sync = 1;
                        }
                    }
                    break;
                }
            }
        } else{
            if (block_bit_cont < 25){
                block_bit_cont++;
            } else{
                int good_block = 0;
                uint64_t dataword = (reg >> 10) & 0xffff;
                uint64_t block_calculated_crc = calc_syndrome(dataword, 16);
                uint64_t checkword = reg & 0x3ff;
                uint64_t block_recieved_crc;
                if (block_number == 2){
                    block_recieved_crc = checkword ^ offset_word[block_number];
                    if (block_recieved_crc == block_calculated_crc){
                        good_block = 1;
                    } else{
                        block_recieved_crc = checkword ^ offset_word[4];
                        if (block_recieved_crc == block_calculated_crc){
                            good_block = 1;
                        } else{
                            wrong_blocks_cont++;
                            good_block = 0;
                        }
                    }
                } else{
                    block_recieved_crc = checkword ^ offset_word[block_number];
                    if (block_recieved_crc == block_calculated_crc){
                        good_block = 1;
                    } else{
                        wrong_blocks_cont++;
                        good_block = 0;
                    }
                }
                uint64_t registr = 0;
                if ((block_number == 0) & good_block){
                    group_assembly_started = 1;
                    group_good_blocks_cont++;
                }
                if (group_assembly_started){
                    if (!good_block){
                        group_assembly_started = 0;
                    } else{
                        registr &= (~(0xFFFF) << (48-block_number*16));
                        registr |= (dataword << (48-block_number*16));
                        group_good_blocks_cont++;
                    }
                    if (group_good_blocks_cont == 5){
                        parse(registr, chars, output, first_time);
                    }
                }
                
                block_bit_cont = 0;
                block_number = (block_number + 1) & 0x03;
                blocks_cont++;
                if (blocks_cont == 50){
                    if (wrong_blocks_cont > 40){
                        std::cerr << "Lost Sync (Got " << wrong_blocks_cont << " bad blocks on " << blocks_cont << " total)" << std::endl;
                        sync = 0;
                        prevsync = 0;
                    } else{ 
                        std::cerr << "Still Sync-ed (Got " << wrong_blocks_cont << " bad blocks on " << blocks_cont << " total)" << std::endl;
                    }
                    blocks_cont = 0;
                    wrong_blocks_cont = 0;
                }
            }
        }
        rds_bit_cont++;
    }
    //std::cerr << rds_bit_cont << std::endl;
    //exit(1);
    
}
