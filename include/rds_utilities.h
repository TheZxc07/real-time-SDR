#include <iostream>
#include <vector>
#include <algorithm>
#include <deque>

int cdr(int, const std::vector<float> &);

void CDR(const std::vector<float> &preCDR, int SPS, int &start, std::vector<int> &state, std::vector<int> &output);

int manchester_united(const std::vector<float> &);

void manchester_decode(std::vector<int> &, const std::vector<int> &, int &, int &, int &);

void differential_decode(std::vector<int> &, const std::vector<int> &, int &, int &);

void error_detection(uint64_t &reg, uint64_t &chars, uint64_t &output, bool &first_time, int &sync, int &prevsync, int &lastseen_offset, int &rds_bit_cont, int &lastseen_offset_cont, int &block_distance, int &block_number, int &block_bit_cont, int &blocks_cont, int &wrong_blocks_cont, int &group_assembly_started, int &group_good_blocks_cont, const std::vector<int> &decoded_bits);

void startSynch(unsigned int &idx, std::vector<int> &stream, std::vector<int> &synch_state_bits, uint64_t &reg, uint64_t &chars, uint64_t &output,bool &first_time, std::deque<std::string> &);
void checkSynch(std::string &offset_type, std::vector<int>::iterator bitstream_start, std::vector<int>::iterator bitstream_end, uint64_t &reg, uint64_t &chars, uint64_t &output,bool &first_time, std::deque<std::string> &); 
