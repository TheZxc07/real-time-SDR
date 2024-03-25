#include "rds_utilities.h"


int cdr(int sps, const std::vector<float> &signal){
    int maxi = 0, maxv = 0, sum = 0;

    for (int i = 0; i < sps; i++){
        for (int k = 500; k < 1000; k++){
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

    for (int i = 0; i < samples.size() - 1; i+=2){
        score += (samples[i]*samples[i+1] > 0); 
    }

    return ~(score >> 31) & 0x01;

}

void manchester_decode(std::vector<int> &bits, const std::vector<float> &samples){
    bits.clear(); bits.resize(samples.size()/2, 0.0);

    for (int i = 0; i < samples.size(); i++){

    }

}

void differential_decode(std::vector<int> &decoded_bits, const std::vector<int> &bits){
    decoded_bits.clear(); decoded_bits.resize(bits.size(), 0.0);

    decoded_bits[0] = bits[0];

    for (int i = 1; i < bits.size(); i++){
        decoded_bits[i] = bits[i]^bits[i-1];
    }
}

int calc_syndrome(int x, int mlen){
    int reg = 0;
    int plen = 10;

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

void decode(){
    int syndrome[] = { 383, 14, 303, 663, 748 };
    int offset_pos[] = { 0, 1, 2, 3, 2 };
    int offset_word[] = { 252, 408, 360, 436, 848 };

    


}

void parse(){
    std::string pty_table[32][2] = {
        {"Undefined", "Undefined"},
        {"News", "News"},
        {"Current Affairs", "Information"},
        {"Information", "Sports"},
        {"Sport", "Talk"},
        {"Education", "Rock"},
        {"Drama", "Classic Rock"},
        {"Culture", "Adult Hits"},
        {"Science", "Soft Rock"},
        {"Varied", "Top 40"},
        {"Pop Music", "Country"},
        {"Rock Music", "Oldies"},
        {"Easy Listening", "Soft"},
        {"Light Classical", "Nostalgia"},
        {"Serious Classical", "Jazz"},
        {"Other Music", "Classical"},
        {"Weather", "Rhythm & Blues"},
        {"Finance", "Soft Rhythm & Blues"},
        {"Children’s Programmes", "Language"},
        {"Social Affairs", "Religious Music"},
        {"Religion", "Religious Talk"},
        {"Phone-In", "Personality"},
        {"Travel", "Public"},
        {"Leisure", "College"},
        {"Jazz Music", "Spanish Talk"},
        {"Country Music", "Spanish Music"},
        {"National Music", "Hip Hop"},
        {"Oldies Music", "Unassigned"},
        {"Folk Music", "Unassigned"},
        {"Documentary", "Weather"},
        {"Alarm Test", "Emergency Test"},
        {"Alarm", "Emergency"}
    };

    int pty_locale = 1;

    std::string coverage_area_codes[16] = {
        "Local",
        "International",
        "National",
        "Supra-regional",
        "Regional 1",
        "Regional 2",
        "Regional 3",
        "Regional 4",
        "Regional 5",
        "Regional 6",
        "Regional 7",
        "Regional 8",
        "Regional 9",
        "Regional 10",
        "Regional 11",
        "Regional 12"
    };


    
}
