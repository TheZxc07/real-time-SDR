
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
        for x in range(500,1000): #10 is arbitrary
            sum += abs(signal[x*SPS + i])
        
        if sum > maxv:
            maxv = sum
            maxi = i
        
        sum = 0
    
    return maxi



audio_Fs = 48e3
audio_decim = 5
# add other settings for audio, like filter taps, ...

# flag that keeps track if your code is running for
# in-lab (il_vs_th = 0) vs takehome (il_vs_th = 1)
il_vs_th = 0



if __name__ == "__main__":

    # read the raw IQ data from the recorded file
    # IQ data is assumed to be in 8-bits unsigned (and interleaved)
    audio_coeff = signal.firwin(rf_taps, [54e3/(5*audio_Fs/2), 60e3/(5*audio_Fs/2)], window=("hann"), pass_zero=False) #given audio_fs is 48, should be 240
    threeKHz_coeff = signal.firwin(rf_taps, 3e3/(5*audio_Fs/2), window=("hann")) #given audio_fs is 48, should be 240
    rrc = impulseResponseRootRaisedCosine(92625, 101)

    d = DFT(audio_coeff)

    #freqzPlot(audio_coeff, 240e3, "")
    #plt.show()

    in_fname = "../data/samples3.raw"
    raw_data = np.fromfile(in_fname, dtype='uint8')
    print("Read raw RF data from \"" + str(len(raw_data)) + "\" in unsigned 8-bit format")
    # IQ data is normalized between -1 and +1 in 32-bit float format
    raw_data = raw_data[:int(len(raw_data)/10)]
    iq_data = (np.float32(raw_data) - 128.0)/128.0
    print("Reformatted raw RF data to 32-bit float format (" + str(iq_data.size * iq_data.itemsize) + " bytes)")

    # set up the subfigures for plotting
    #subfig_height = np.array([0.8, 2, 1.6]) # relative heights of the subfigures
    #plt.rc('figure', figsize=(7.5, 7.5))	# the size of the entire figure
    #fig, (ax0, ax1, ax2) = plt.subplots(nrows=3, gridspec_kw={'height_ratios': subfig_height})
    #fig.subplots_adjust(hspace = .6)

    # coefficients for the front-end low-pass filter
    rf_coeff = signal.firwin(rf_taps, rf_Fc/(rf_Fs/2), window=('hann'))

    # filter to extract the FM channel (I samples are even, Q samples are odd)
    i_filt = signal.lfilter(rf_coeff, 1.0, iq_data[0::2])
    q_filt = signal.lfilter(rf_coeff, 1.0, iq_data[1::2])

    # downsample the FM channel
    i_ds = i_filt[::rf_decim]
    q_ds = q_filt[::rf_decim]


    fm_demod, dummy = fmDemodArctan(i_ds, q_ds)

    audio_filt = signal.lfilter(audio_coeff, 1.0, fm_demod)

    #####START OF RDS#######

    linSquare = np.square(audio_filt)

    pll_clean_coeff = signal.firwin(rf_taps, [113.5e3/(5*audio_Fs/2), 114.5e3/(5*audio_Fs/2)], window=("hann"), pass_zero=False) #given audio_fs is 48, should be 240
    genPilot = signal.lfilter(pll_clean_coeff, 1.0, linSquare)
    # PSD after extracting mono audio


    IPll, QPll = fmPll(genPilot, 114e3, 240e3, 0.5, -0*np.pi/8, 0.001)
    time0 = np.arange(0,50)

    #plt.plot(time0, IPll[:50])
    #plt.plot(time0, QPll[:50], color = "green")
    #plt.show()
    #QPll = fmPll(genPilot, 114e3, 240e3, 0.5, np.pi/2, 0.01)

    def APF(taps, loc):
        taps = np.zeros(taps)
        taps[50] = 1

        return taps

    rdsAPF = APF(rf_taps, 50)

    data_delayed = signal.lfilter(rdsAPF, 1.0, audio_filt) 

    mixed = 2*data_delayed*IPll[:-1]
    clean = signal.lfilter(threeKHz_coeff, 1.0, mixed)

    clean = resample_poly(clean, 247, 640)
    clean = signal.lfilter(rrc, 1.0, clean)

    #################DO IT AGAIN FOR Q ##########################

    Qmixed = 2*data_delayed*QPll[:-1]
    Qclean = signal.lfilter(threeKHz_coeff, 1.0, Qmixed)

    Qclean = resample_poly(Qclean, 247, 640)
    Qclean = signal.lfilter(rrc, 1.0, Qclean)

    time1 = np.arange(0,500)

    #plt.plot(time1, clean[:500])
    #plt.plot(time1, Qclean[:500], color = "green")
    #plt.show()


    #plt.scatter(clean[10000:11000], Qclean[10000:11000])
    #plt.scatter(clean[20000:21000], Qclean[20000:21000])

    plt.show()

    offest = CDR(39, clean)
    print(offest)

    SymbArr = clean[offest::39] #39 is SPS
    QSymbArr = Qclean[offest::39]

    time = np.arange(0, 50)
    time1 = np.arange(0,100)

    #fmPlotPSD(ax1, clean, (rf_Fs/rf_decim)/1e3, subfig_height[1], 'Extracted Mono')
    plotTime(SymbArr[10:60], time)
    plt.show()
    plt.scatter(SymbArr, QSymbArr, s=10)
    plt.show()

    #plotTime(Qclean[1000:6000], time)

    #plotTime(SymbArr[0:100], time1)



    # save PSD plots
    fig.savefig("../data/fmMonoBasic.png")
    plt.show()

    #linSquare = np.square(audio_filt)

