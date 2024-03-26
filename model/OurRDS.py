
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import numpy as np
import math
from fmPll import *

# use fmDemodArctan and fmPlotPSD
from fmSupportLib import *
from fmRRC import *
from fourierTransform import *
from scipy.signal import *
# for take-home add your functions

rf_Fs = 2.4e6
rf_Fc = 100e3
rf_taps = 101
rf_decim = 10

def resample():
    pass

def CDR(SPS, signal):
    maxi, maxv, sum = 0,0, 0 #max index, max value
    for i in range(SPS):
        for x in range(10): #10 is arbitrary
            sum += abs(signal[x*SPS + i])
        
        if sum > maxv:
            maxv = sum
            maxi = i
        
        sum = 0
    
    return maxi


def ManDecode(samples, block_count : int,  halfSymbol : int, start : int = 0):
    bits = [] #will  have length half of samples
    #print(len(samples))

    #if this block is starting at 1, use the previous blocks last and this ones 0
    if start:
        bits.append(halfSymbol) #first half of man symbol = symbol

    if block_count == 0:
        score = 0
        #brute force, check with each offset
        for i in range(0, len(samples)-1, 2):
            score += samples[i] ^ samples[i+1]

        for i in range(1, len(samples)-1, 2):
            score -= samples[i] ^ samples[i+1]
        start = score < 0

    #start = ManUtd(samples)
    
    for i, n in enumerate(samples[start:-1:2]):
        bits.append( n )

    if ((len(samples) - start) % 2 == 1): #if a leftover bit
        halfSymbol = samples[-1]
        start = True
    else:
        start = False

    return bits, halfSymbol, start

def diffDecode(bits, last, block_num):
    decoded = [] #length is equal bits
    if (block_num == 0):
        decoded.append(bits[0])
    else:
        decoded.append(bits[0]^last)
    #print(bits)
    for i in range(1, len(bits)):
        decoded.append(bits[i]^bits[i-1])

    return decoded, bits[-1]

def APF(taps, loc):
    taps = np.zeros(taps)
    taps[50] = 1

    return taps



audio_Fs = 48e3
audio_decim = 5
# add other settings for audio, like filter taps, ...

# flag that keeps track if your code is running for
# in-lab (il_vs_th = 0) vs takehome (il_vs_th = 1)
il_vs_th = 0

audio_coeff = signal.firwin(rf_taps, [54e3/(5*audio_Fs/2), 60e3/(5*audio_Fs/2)], window=("hann"), pass_zero=False) #given audio_fs is 48, should be 240
threeKHz_coeff = signal.firwin(rf_taps, 3e3/(5*audio_Fs/2), window=("hann")) #given audio_fs is 48, should be 240
rrc = impulseResponseRootRaisedCosine(92625, 101)

# coefficients for the front-end low-pass filter
rf_coeff = signal.firwin(rf_taps, rf_Fc/(rf_Fs/2), window=('hann'))
pll_clean_coeff = signal.firwin(rf_taps, [113.5e3/(5*audio_Fs/2), 114.5e3/(5*audio_Fs/2)], window=("hann"), pass_zero=False) #given audio_fs is 48, should be 240


rdsAPF = APF(rf_taps, 50)

# Constants
syndrome = [383, 14, 303, 663, 748]
offset_pos = [0, 1, 2, 3, 2]
offset_word = [252, 408, 360, 436, 848]

# see Annex B, page 64 of the standard
def calc_syndrome(x, mlen):
    reg = 0
    plen = 10
    for ii in range(mlen, 0, -1):
        reg = (reg << 1) | ((x >> (ii-1)) & 0x01)
        if (reg & (1 << plen)):
            reg = reg ^ 0x5B9
    for ii in range(plen, 0, -1):
        reg = reg << 1
        if (reg & (1 << plen)):
            reg = reg ^ 0x5B9
    return reg & ((1 << plen) - 1) # select the bottom plen bits of reg

def stringify(inp: np.uint64)-> str: #pure helper for visualiztion
    s = ""
    for n in range(8) : #8 bytes
        s = chr((inp >> np.uint64(n*8)) & np.uint64(0xFF)) + s

    return s



def parse(bytes : np.uint64, first_time : bool, PS_chars : np.uint64, output : np.uint64):
    group_type = (bytes >> np.uint64(44)) & np.uint64(0xf) #44 = 32 for the first two groups, plus 12 to get group type 
    AB = (bytes >> np.uint64(43)) &np.uint64( 0x1 ) #1 means B block, 0 means A block
    placement = (bytes >> np.uint64(32)) & np.uint64(0x03) #isolate two lsb of second group
    PI = (bytes >> np.uint64(48)) & np.uint64(0xFFFF)
    PTY = (bytes >> np.uint64(37)) & np.uint64(0x1f)
    PTY_decode = pty_table[PTY][1]

    if first_time:
        print(f"PI: {hex(PI)}")
        print(f"PTY: {PTY_decode}")
        first_time = False

    
    if group_type == 0:#we don't care about A/B, because the last little grop is the same
        #print(chars)
        PS_chars = np.left_shift(PS_chars, np.uint64(16)) | (bytes & np.uint64(0xFFFF))
        #chars[2*placement+1] = chr(bytes[7])
        #chars[2*placement] = chr(bytes[6])

        if placement == 3 and PS_chars != output:#if at the end of four of these, we have a new word
            output = PS_chars #set the state
            print(f'PS: {stringify(output)}')

    return PS_chars, output, first_time

# Annex F of RBDS Standard Table F.1 (North America) and Table F.2 (Europe)
#              Europe                   North America
pty_table = [["Undefined",             "Undefined"],
            ["News",                  "News"],
            ["Current Affairs",       "Information"],
            ["Information",           "Sports"],
            ["Sport",                 "Talk"],
            ["Education",             "Rock"],
            ["Drama",                 "Classic Rock"],
            ["Culture",               "Adult Hits"],
            ["Science",               "Soft Rock"],
            ["Varied",                "Top 40"],
            ["Pop Music",             "Country"],
            ["Rock Music",            "Oldies"],
            ["Easy Listening",        "Soft"],
            ["Light Classical",       "Nostalgia"],
            ["Serious Classical",     "Jazz"],
            ["Other Music",           "Classical"],
            ["Weather",               "Rhythm & Blues"],
            ["Finance",               "Soft Rhythm & Blues"],
            ["Children’s Programmes", "Language"],
            ["Social Affairs",        "Religious Music"],
            ["Religion",              "Religious Talk"],
            ["Phone-In",              "Personality"],
            ["Travel",                "Public"],
            ["Leisure",               "College"],
            ["Jazz Music",            "Spanish Talk"],
            ["Country Music",         "Spanish Music"],
            ["National Music",        "Hip Hop"],
            ["Oldies Music",          "Unassigned"],
            ["Folk Music",            "Unassigned"],
            ["Documentary",           "Weather"],
            ["Alarm Test",            "Emergency Test"],
            ["Alarm",                 "Emergency"]]
pty_locale = 1 # set to 0 for Europe which will use first column instead

# page 72, Annex D, table D.2 in the standard
coverage_area_codes = ["Local",
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
                    "Regional 12"]


if __name__ == "__main__":

    # read the raw IQ data from the recorded file
    # IQ data is assumed to be in 8-bits unsigned (and interleaved)


    #d = DFT(audio_coeff)

    #freqzPlot(audio_coeff, 240e3, "")
    #plt.show()

    # select a block_size that is a multiple of KB
    # and a multiple of decimation factors
    block_size = 1024 * rf_decim * audio_decim * 2
    block_count = 0

    # states needed for continuity in block processing
    state_i_lpf_100k = np.zeros(rf_taps-1)
    state_q_lpf_100k = np.zeros(rf_taps-1)
    state_phase = 0

    # add state as needed for the mono channel filter
    audio_taps = 101
    state_mono = np.zeros(audio_taps - 1)

    # audio buffer that stores all the audio blocks
    audio_data = np.array([]) # used to concatenate filtered blocks (audio data)

    offest = 0

    state_phase = 0.0
    audio_state = np.zeros(audio_taps - 1)
    state_mono = np.zeros(audio_taps - 1)
    pilot_state = np.zeros(audio_taps - 1)
    delay_state = np.zeros(audio_taps - 1)
    baseband_state = np.zeros(audio_taps - 1)
    amplified_state = np.zeros(audio_taps - 1)
    pll_state = [0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0]

    manDecodeHalfSymbol = 0
    manBlockStart = 0

    man = [] # for testing only
    S = []

    rdsBitCtr = 0

    decodeBuffer = 0 #buffer between decode blocks

    ###FOR DECODER
        # Initialize all the working vars we'll need during the loop
    synced = False
    presync = False

    wrong_blocks_counter = 0
    blocks_counter = 0
    group_good_blocks_counter = 0

    reg = np.uint32(0) # was unsigned long in C++ (64 bits) but numpy doesn't support bitwise ops of uint64, I don't think it gets that high anyway
    lastseen_offset_counter = 0
    lastseen_offset = 0

    bytes_out = []

    chars = np.uint64(0)
    output = np.uint64(0)
    first_time = True

    test_arr = []


    in_fname = "./data/samples8.raw"
    raw_data = np.fromfile(in_fname, dtype='uint8')
    print("Read raw RF data from \"" + str(len(raw_data)) + "\" in unsigned 8-bit format")
    # IQ data is normalized between -1 and +1 in 32-bit float format
    iq_data = (np.float32(raw_data) - 128.0)/128.0
    print("Reformatted raw RF data to 32-bit float format (" + str(iq_data.size * iq_data.itemsize) + " bytes)")
    bits = []
    print(len(iq_data))

    while (block_count+1)*block_size < len(iq_data):


        # set up the subfigures for plotting
        #subfig_height = np.array([0.8, 2, 1.6]) # relative heights of the subfigures
        #plt.rc('figure', figsize=(7.5, 7.5))	# the size of the entire figure
        #fig, (ax0, ax1, ax2) = plt.subplots(nrows=3, gridspec_kw={'height_ratios': subfig_height})
        #fig.subplots_adjust(hspace = .6)

        #print('Processing block ' + str(block_count))

        # filter to extract the FM channel (I samples are even, Q samples are odd)
        i_filt, state_i_lpf_100k = signal.lfilter(rf_coeff, 1.0, \
                iq_data[(block_count)*block_size:(block_count+1)*block_size:2],
                zi=state_i_lpf_100k)
        q_filt, state_q_lpf_100k = signal.lfilter(rf_coeff, 1.0, \
                iq_data[(block_count)*block_size+1:(block_count+1)*block_size:2],
                zi=state_q_lpf_100k)

        ## filter to extract the FM channel (I samples are even, Q samples are odd)
        #i_filt = signal.lfilter(rf_coeff, 1.0, iq_data[0::2])
        #q_filt = signal.lfilter(rf_coeff, 1.0, iq_data[1::2])

        # downsample the FM channel
        i_ds = i_filt[::rf_decim]
        q_ds = q_filt[::rf_decim]


        fm_demod, state_phase= fmDemodArctan(i_ds, q_ds, state_phase)

        audio_filt, audio_state = signal.lfilter(audio_coeff, 1.0, fm_demod, zi=audio_state)

        #####START OF RDS#######

        linSquare = np.square(audio_filt)


        genPilot, pilot_state = signal.lfilter(pll_clean_coeff, 1.0, linSquare, zi=pilot_state)
        # PSD after extracting mono audio


        IPll, QPll, pll_state = fmPll(genPilot, 114e3, 240e3, pll_state, 0.5, -0*np.pi/8, 0.001)
        time0 = np.arange(0,50)

        #plt.plot(time0, IPll[:50])
        #plt.plot(time0, QPll[:50], color = "green")
        #plt.show()
        #QPll = fmPll(genPilot, 114e3, 240e3, 0.5, np.pi/2, 0.01)
        
        data_delayed, delay_state = signal.lfilter(rdsAPF, 1.0, audio_filt, zi = delay_state) 

        mixed = 2*data_delayed*IPll[:-1]
        baseband, baseband_state = signal.lfilter(threeKHz_coeff, 1.0, mixed, zi = baseband_state)

        #####COMBINE THE FOLLOWING TWO IN THE C++#####
        baseband = resample_poly(baseband, 247, 640)
        amplified, amplified_state = signal.lfilter(rrc, 1.0, baseband, zi = amplified_state)

        #################DO IT AGAIN FOR Q ##########################

        #Qmixed = 2*data_delayed*QPll[:-1]
        #Qclean = signal.lfilter(threeKHz_coeff, 1.0, Qmixed)
        #Qclean = resample_poly(Qclean, 247, 640)
        #Qclean = signal.lfilter(rrc, 1.0, Qclean)
        #time1 = np.arange(0,150)
        #plt.plot(time1, clean[:150])
        #plt.plot(time1, Qclean[:150], color = "green")
        #plt.show()


        #plt.scatter(clean[10000:11000], Qclean[10000:11000])
        #plt.scatter(clean[20000:21000], Qclean[20000:21000])

        #plt.show()
        offest = CDR(39, amplified)
        #print(offest)

        SymbArr = amplified[offest::39] > 0 #39 is SPS
        S += list(SymbArr>0)
        #QSymbArr = Qclean[offest::39]

#       time = np.arange(0, 50)
#       time1 = np.arange(0,100)

        manDec, manDecodeHalfSymbol, manBlockStart = ManDecode(SymbArr, block_count, manDecodeHalfSymbol, manBlockStart)
        if block_count == 1:
            print(manDec)
        man += manDec


        decodeBlk, decodeBuffer = diffDecode(manDec, decodeBuffer, block_count)

        bits += decodeBlk
        

        block_count += 1

    ###########
    # DECODER #
    ###########


    #print(bits[:100])
    #print(man)

        for i in range(len(decodeBlk)):
            # in C++ reg doesn't get init so it will be random at first, for ours its 0s
            # It was also an unsigned long but never seemed to get anywhere near the max value
            # bits are either 0 or 1
            reg = np.bitwise_or(np.left_shift(reg, 1), decodeBlk[i]) # reg contains the last 26 rds bits. these are both bitwise ops
            if not synced:
                reg_syndrome = calc_syndrome(reg, 26)
                #print(decodeBlk[i], end = " ")
                #test_arr.append(reg_syndrome)
                #print(reg_syndrome, end=" ")
                for j in range(5):
                    #test_arr.append(block_distance)
                    if reg_syndrome == syndrome[j]:
                        if not presync:
                            lastseen_offset = j
                            lastseen_offset_counter = rdsBitCtr
                            presync = True
                        else:
                            
                            if offset_pos[lastseen_offset] >= offset_pos[j]:
                                block_distance = offset_pos[j] + 4 - offset_pos[lastseen_offset]
                            else:
                                block_distance = offset_pos[j] - offset_pos[lastseen_offset]
                            if (block_distance*26) != (rdsBitCtr - lastseen_offset_counter):
                                presync = False
                            else:
                                print('Sync State Detected')
                                wrong_blocks_counter = 0
                                blocks_counter = 0
                                block_bit_counter = 0
                                block_number = (j + 1) % 4
                                group_assembly_started = False
                                synced = True
                        break # syndrome found, no more cycles

            else: # SYNCED
                # wait until 26 bits enter the buffer */
                #print('hihi')
                if block_bit_counter < 25:
                    block_bit_counter += 1
                else:
                    good_block = False
                    dataword = (reg >> 10) & 0xffff
                    block_calculated_crc = calc_syndrome(dataword, 16)
                    checkword = reg & 0x3ff
                    if block_number == 2: # manage special case of C or C' offset word
                        block_received_crc = checkword ^ offset_word[block_number]
                        if (block_received_crc == block_calculated_crc):
                            good_block = True
                        else:
                            block_received_crc = checkword ^ offset_word[4]
                            if (block_received_crc == block_calculated_crc):
                                good_block = True
                            else:
                                wrong_blocks_counter += 1
                                good_block = False
                    else:
                        block_received_crc = checkword ^ offset_word[block_number] # bitwise xor
                        if block_received_crc == block_calculated_crc:
                            good_block = True
                        else:
                            wrong_blocks_counter += 1
                            good_block = False

                    # Done checking CRC
                    if block_number == 0 and good_block:
                        group_assembly_started = True
                        group_good_blocks_counter = 1
                        bytes = bytearray(8) # 8 bytes filled with 0s
                        register = np.uint64(0)

                    if group_assembly_started:
                        if not good_block:
                            group_assembly_started = False
                        else:
                            # raw data bytes, as received from RDS. 8 info bytes, followed by 4 RDS offset chars: ABCD/ABcD/EEEE (in US) which we leave out here
                            # RDS information words
                            # block_number is either 0,1,2,3 so this is how we fill out the 8 bytes
                            bytes[block_number*2] = (dataword >> 8) & 255
                            bytes[block_number*2+1] = dataword & 255
                            register &= ~(np.uint64(0xFFFF) << np.uint64(48-block_number*16))
                            register |= np.uint64(dataword) << np.uint64(48 - block_number*16)
                            group_good_blocks_counter += 1
                            #print('group_good_blocks_counter:', group_good_blocks_counter)
                        if group_good_blocks_counter == 5:
                            #print(bytes)
                            bytes_out.append(bytes) # list of len-8 lists of bytes
                            chars, output, first_time = parse(register, first_time, chars, output)


                    block_bit_counter = 0
                    block_number = (block_number + 1) % 4
                    blocks_counter += 1
                    if blocks_counter == 50:
                        if wrong_blocks_counter > 10: # This many wrong blocks must mean we lost sync
                            print("Lost Sync (Got ", wrong_blocks_counter, " bad blocks on ", blocks_counter, " total)")
                            synced = False
                            presync = False
                        else:
                            print("Still Sync-ed (Got ", wrong_blocks_counter, " bad blocks on ", blocks_counter, " total)")
                        blocks_counter = 0
                        wrong_blocks_counter = 0

            rdsBitCtr += 1 #sub for i
    #print(test_arr)

    ###########
    # PARSER  #
    ###########
    '''
    print(bytes_out)
    bytes_out = bytes_out[:]
    first_time = True
    chars = [" "]*8
    output = "        "
    for bytes in bytes_out:
        group_0 = bytes[1] | (bytes[0] << 8)
        group_1 = bytes[3] | (bytes[2] << 8)
        group_2 = bytes[5] | (bytes[4] << 8)
        group_3 = bytes[7] | (bytes[6] << 8)

        group_type = (group_1 >> 12) & 0xf # here is what each one means, e.g. RT is radiotext which is the only one we decode here: ["BASIC", "PIN/SL", "RT", "AID", "CT", "TDC", "IH", "RP", "TMC", "EWS", "___", "___", "___", "___", "EON", "___"]
        AB = (group_1 >> 11 ) & 0x1 # b if 1, a if 0
        placement = (group_1) &0x03

        #print("group_type:", group_type) # this is essentially message type, i only see type 0 and 2 in my recording
        #print("AB:", AB)

        program_identification = group_0     # "PI"

        program_type = (group_1 >> 5) & 0x1f # "PTY"
        pty = pty_table[program_type][pty_locale]

        pi_area_coverage = (program_identification >> 8) & 0xf
        coverage_area = coverage_area_codes[pi_area_coverage]

        pi_program_reference_number = program_identification & 0xff # just an int

        if first_time:
            print("PTY:", pty)
            print("program:", hex(program_identification))
            print("coverage_area:", coverage_area)
            first_time = False


        
        if group_type == 0:
            #print(chars)
            chars[2*placement+1] = chr(bytes[7])
            chars[2*placement] = chr(bytes[6])

            if placement == 3 and "".join(chars) != output:
                output = "".join(chars)
                print(f'PS: {output}')
            #print(f'PS: {x}, {placement}')
'''
            
'''
        if group_type == 2:
            # when the A/B flag is toggled, flush your current radiotext
            if radiotext_AB_flag != ((group_1 >> 4) & 0x01):
                radiotext = [' ']*65
            radiotext_AB_flag = (group_1 >> 4) & 0x01
            text_segment_address_code = group_1 & 0x0f
            if AB:
                radiotext[text_segment_address_code * 2    ] = chr((group_3 >> 8) & 0xff)
                radiotext[text_segment_address_code * 2 + 1] = chr(group_3        & 0xff)
            else:
                radiotext[text_segment_address_code *4     ] = chr((group_2 >> 8) & 0xff)
                radiotext[text_segment_address_code * 4 + 1] = chr(group_2        & 0xff)
                radiotext[text_segment_address_code * 4 + 2] = chr((group_3 >> 8) & 0xff)
                radiotext[text_segment_address_code * 4 + 3] = chr(group_3        & 0xff)
            print(''.join(radiotext))
        else:
            pass
            #print("unsupported group_type:", group_type)'''

    #fmPlotPSD(ax1, clean, (rf_Fs/rf_decim)/1e3, subfig_height[1], 'Extracted Mono')
    #plotTime(SymbArr[10:60], time)
    #plt.show()
    #plt.scatter(SymbArr, QSymbArr, s=10)
    #plt.show()

    #plotTime(Qclean[1000:6000], time)

    #plotTime(SymbArr[0:100], time1)



    # save PSD plots
    #fig.savefig("../data/fmMonoBasic.png")
    #plt.show()

    #linSquare = np.square(audio_filt)

