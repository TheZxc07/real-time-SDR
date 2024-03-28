#include <iostream>
#include <stdint.h>
#include <stdlib.h>


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


void stringify(uint64_t characters, char* combined){
    //char combined[9]; //one char for null termination
    
    for (int i = 0; i < 8; i++ ){
        combined[7-i] = (characters >> (i << 3)) & 0xFF; 
    }
    combined [8] = 0x0;
    
}


void parse (const uint64_t &bytes, uint64_t &chars, uint64_t &output, bool &first_time){
    uint8_t group_type = (bytes >> 44) &0xF;
    uint8_t AB_flag = (bytes >> 43) & 0x1;
    uint8_t placement = (bytes >> 32) & 0x3;
    uint16_t PI = (bytes >> 48) & 0xFFFF;
    uint8_t PTY_data = (bytes >> 37) & 0x1F;
    
    if (first_time){
        std::cout << "PI: " << PI << std::endl;
        std::cout << "PTY: " << PTY_codes[PTY_data] << std::endl;
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

int main(int argc, char* argv[]){
    uint64_t regs[] = {
    14013513924445631589,
    14013549063265283872,
    14013513945920467571,
    14013654642130354176,
    14013513915855685225,
    14013549067559855976,
    14013531469908082688,
    14013654642130354176,
    14013513920150660968,
    14013549072626315891,
    14013566722989542726,
    14013654642130354176,
    14013513915855675424,
    14013549075508305996,
    14013513920150654063,
    14013654663698388830,
    14013513924445632101,
    14013549081133475104,
    14013513945920446496,
    14013654663698388830,
    14013513915855675424,
    14013549084706170227,
    14013513920150654063,
    14013654663698388830,
    14013513924445632101,
    14013549088396111733,
    14013513945920446496,
    14013654663698386240,
    14013513915855675424,
    14013549094000594208,
    14013513920150654063,
    14013654663698386240,
    14013513915855675424,
    14013549063265283872,
    14013531469908082688,
    14013654663698386240,
    14013513920150652009,
    14013549067559855976,
    14013566724093037894,
    14013654658329710035,
    14013513924445627763,
    14013549072626315891,
    14013513945920446496,
    14013654658329710035,
    14013513915855675424,
    14013549075508305996,
    14013513920150652009,
    14013654658329710035,
    14013513924445627763,
    14013549081133475104,
    14013513945920446496,
    14013654663700715284,
    14013513915855675424,
    14013549084706170227,
    14013513920150652009,
    14013654663700715284
    };

    uint64_t chars = 0;
    uint64_t output = 0;
    bool first_time = true;

    for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++){
        parse(regs[i], chars, output, first_time);
    }


}