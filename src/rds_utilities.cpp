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


const std::vector<std::vector<bool>> parity_matrix = {
     {1,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,1,1,1},
     {0,1,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,1,1},
     {0,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,0,0,1,0,1,1,1,1,1,0},
     {0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,0,0,0},
     {0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,0,0},
     {0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,1},
     {0,0,0,0,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,0,1,1,0,0,1,1},
     {0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,1,1,1,1,1,0},
     {0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,1,1,1,1,1},
     {0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,1,1,1,1} 
};

const std::vector<bool> syndrome_bits = { 1,1,1,1,0,1,1,0,0,0,1,1,1,1,0,1,0,1,0,0,1,0,0,1,0,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0 };

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
    
    if (1){
        std::cerr << "PI: " << std::hex << PI << std::endl;
        std::cerr << "PTY: " << PTY_codes[PTY_data] << std::endl;
        first_time = false;
    }
    
    
    if (group_type == 0){
        uint64_t mask = ~(static_cast<uint64_t>(0xFFFF) << (48-16*placement));
        chars = chars & mask;
        uint64_t word = bytes & static_cast<uint64_t>(0xFFFF);
        chars = chars | (static_cast<uint64_t>(word) << 16*(3-placement)); //shift up characters, add the new ones from the last block
        //note, for 0A or 0B, the last 16 data bits are unchanged
        if ((placement == 3) && (chars != output)){ //if we've filled 8 characters
            output = chars; //update state
            char str[9];
            stringify(chars, str);
            std::cerr << "Program Service: " << str << std::endl;
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
            std::cerr << "Reg Syndrome: " << reg_syndrome << "    Reg: " << reg << std::endl;
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

void uint_copy(uint64_t &reg, std::vector<int>::iterator bitstream_start, std::vector<int>::iterator bitstream_end, int block_type){
    //types A = 0, B = 1, C = 2, D = 3
    
    std::vector<int> scan(bitstream_start, bitstream_end);
    
    uint64_t mask = ~(static_cast<uint64_t>(0xFFFF) << (48-16*block_type));
    
    /*
    if (block_type == 0){
        for (int i = 0; i < scan.size(); i++){
            std::cerr << scan[i] << " ";
        }
    }
    std::cerr << std::endl;
    */
    reg = reg & mask;
    for(int i = 0; i < scan.size(); i++){
        reg = reg | (static_cast<uint64_t>(scan[i]) << (15 - i + 48 - 16*block_type));
    }
    /*
    std::cerr << "Mask: " << std::hex << mask << std::endl; 
    std::cerr << "Block detected: " << std::hex << reg << std::endl;
    */
    
}   

bool isSequenceABCD(std::string current, std::deque<std::string> &window) {
    window.push_back(current);
    if (window.size() > 4)
        window.pop_front();

    // Check if the current window forms the sequence A->B->C->D
    if (window.size() == 4 && window[0] == "A" && window[1] == "B" && window[2] == "C" && window[3] == "D") {
        return true; // Sequence found
    }

    return false; // Sequence not found
}

void check_block(std::string &offset_type, std::vector<int>::iterator bitstream_start, std::vector<int>::iterator bitstream_end, uint64_t &reg, uint64_t &chars, uint64_t &output,bool &first_time, std::deque<std::string> &window) {
    std::vector<bool> parity_vec(26); 
    std::vector<bool> prod(26, false); 
    std::vector<bool> syndrome_vec(10); 

    for (int col = 0; col < 10; col++) {

        std::transform(bitstream_start, bitstream_end, parity_matrix[col].begin(), prod.begin(), std::multiplies<bool>());
		
        syndrome_vec[col] = (std::count(prod.begin(), prod.end(), true) % 2) == 1;
    }
    
    for (unsigned int idx = 0; idx <= 40; idx += 10) {
        if (std::equal(syndrome_bits.begin() + idx, syndrome_bits.begin() + idx + 10, syndrome_vec.begin())) {
            switch (idx) {
                case 0: { offset_type = "A"; uint_copy(reg, bitstream_start, bitstream_end -10, 0); break; }
                case 10:{ offset_type = "B"; uint_copy(reg, bitstream_start, bitstream_end -10, 1); break; }
                case 20:{ offset_type = "C"; uint_copy(reg, bitstream_start, bitstream_end -10, 2); break; }
                case 30: offset_type = "Cp"; break;
                case 40:{ offset_type = "D"; uint_copy(reg, bitstream_start, bitstream_end -10, 3);  break;}
            }
            if (isSequenceABCD(offset_type, window)){
                parse(reg, chars, output, first_time);
            }

            return;
        }
    }
    offset_type = "None";
}


void start_frame_sync(unsigned int &idx, std::vector<int> &decoded_stream, std::vector<int> &sync_state_bits, uint64_t &reg, uint64_t &chars, uint64_t &output,bool &first_time, std::deque<std::string> &window) {
    decoded_stream.insert(decoded_stream.begin(), sync_state_bits.begin(), sync_state_bits.end());
    
    std::string x = "";
    unsigned int end_range = decoded_stream.size() - 26;
 
	while (idx < end_range) {        
        check_block(x, decoded_stream.begin()+idx, decoded_stream.begin()+idx+26, reg, chars, output, first_time, window);
        if(x != "None"){
			idx += 26;
		}
		else
			idx += 1;
    }
	sync_state_bits.clear();
	sync_state_bits.insert(sync_state_bits.begin(), decoded_stream.begin() + idx, decoded_stream.end());
}

