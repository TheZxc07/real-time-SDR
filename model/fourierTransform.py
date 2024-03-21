#
# Comp Eng 3DY4 (Computer Systems Integration Project)
#
# Copyright by Nicola Nicolici
# Department of Electrical and Computer Engineering
# McMaster University
# Ontario, Canada
#

import matplotlib.pyplot as plt
import numpy as np
import cmath, math
from random import randint
from scipy import signal
import scipy
import sys
def plotSpectrum(x, Fs, type = 'FFT', inputArr = None):

    n = len(x)             # length of the signal
    df = Fs/n              # frequency increment (width of freq bin)

    # compute Fourier transform, its magnitude and normalize it before plotting
    if type == 'FFT':
        Xfreq = np.fft.fft(x)
    elif type == "il1":
        Xfreq = np.array( inputArr )
    XMag = abs(Xfreq)/n


    # Note: because x is real, we keep only the positive half of the spectrum
    # Note also: half of the energy is in the negative half (not plotted)
    XMag = XMag[0:int(n/2)]

    # freq vector up to Nyquist freq (half of the sample rate)
    freq = np.arange(0, Fs/2, df)

    fig, ax = plt.subplots()
    ax.plot(freq, XMag)
    ax.set(xlabel='Frequency (Hz)', ylabel='Magnitude',
        title='Frequency domain plot')
    # fig.savefig("freq.png")
    plt.show()

def plotTime(x, time):

    fig, ax = plt.subplots()
    ax.plot(time, x)
    ax.set(xlabel='Time (sec)', ylabel='Amplitude',
            title='Time domain plot')
    # fig.savefig("time.png")
    plt.show()

def generateSin(Fs, interval, frequency = 7.0, amplitude = 5.0, phase = 0.0):

    dt = 1.0/Fs                          # sampling period (increment in time)
    time = np.arange(0, interval, dt)    # time vector over interval

    # generate the sin signal
    x = amplitude*np.sin(2*math.pi*frequency*time+phase)

    return time, x

def generateSquare(Fs, interval, frequency = 7.0, amplitude = 5.0, dutyCycle = 0.5):
    dt = 1.0/Fs                          # sampling period (increment in time)
    time = np.arange(0, interval, dt)    # time vector over interval

    #genrate the squareWave
    x=amplitude*signal.square(2*math.pi*frequency*time, dutyCycle)

    return time, x

def genMultiTone(Fs : float, interval : float):
    dt = 1.0/Fs                          # sampling period (increment in time)
    time = np.arange(0, interval, dt)    # time vector over interval

    freqs = 10*np.random.randn(3)
    amps = 10*np.random.randn(3)
    phases = 10*np.random.randn(3)
    # generate the sin signal
    #time, y = generateSin(freqs[0],amps[0], phases[0])
    #x = y
    #time, y = generateSin(freqs[1],amps[1], phases[1])
    #x += y
    #time, y = generateSin(freqs[2],amps[2], phases[2])
    #x += y
      # generate the sin signals and sum them up
    _, y1 = generateSin(Fs, interval, freqs[0], amps[0], phases[0])
    _, y2 = generateSin(Fs, interval, freqs[1], amps[1], phases[1])
    _, y3 = generateSin(Fs, interval, freqs[2], amps[2], phases[2])

    x = y1 + y2 + y3



    return time, x

def genFourTone(Fs : float, interval : float):
    dt = 1.0/Fs                          # sampling period (increment in time)
    time = np.arange(0, interval, dt)    # time vector over interval

    freqs = 10*np.random.randn(4)
    amps = 10*np.random.randn(4)
    phases = 10*np.random.randn(4)
    # generate the sin signal
    #time, y = generateSin(freqs[0],amps[0], phases[0])
    #x = y
    #time, y = generateSin(freqs[1],amps[1], phases[1])
    #x += y
    #time, y = generateSin(freqs[2],amps[2], phases[2])
    #x += y
      # generate the sin signals and sum them up
    _, y1 = generateSin(Fs, interval, 5, 5, 1)
    _, y2 = generateSin(Fs, interval, 20, 8, 6)
    _, y3 = generateSin(Fs, interval, 35, 10, 4)
    _, y4 = generateSin(Fs, interval, 47,7, 2)

    x = y1 + y2 + y3 + y4



    return time, x





def cli_error_msg():

    # error message to provide the correct command line interface (CLI) arguments
    print('Valid arguments:')
    print('\trc:  reference code')
    print('\til1: in-lab 1')
    print('\til2: in-lab 2')
    print('\til3: in-lab 3')
    print('\tth:  take-home')
    sys.exit()


def dft(x):

    X = []
    N = len(x)
    for m in range(0, N):
        X.append(0)
        for k in range (0, N):
            X[m] = X[m] + (x[k] *cmath.exp(-2j*math.pi*k*m/N))

    #print(X)

    return X

def idft(X):
    x = []
    N = len(X)
    for k in range(0, N):
        x.append(0)
        for m in range (0, N):
            x[k] = x[k] + (X[m] *cmath.exp(2j*math.pi*k*m/N))
    return x

if __name__ == "__main__":

    if len(sys.argv[0:]) != 2:
        cli_error_msg()

    Fs = 100.0          # sampling rate
    interval = 1.0      # set up to one full second

    if (sys.argv[1] == 'rc'): # runs the reference code (rc)

        print('Reference code for the Fourier transform')

        # generate the user-defined sin function
        time, x = generateSin(Fs, interval)
        # plot the signal in time domain
        plotTime(x, time)
        # plot the signal in frequency domain
        plotSpectrum(x, Fs, type = 'FFT')

    elif (sys.argv[1] == 'il1'):

        print('In-lab experiment 1 for the Fourier transform')

        time, x = generateSin(Fs, interval)

        X = dft(x)

        plotSpectrum(x, Fs, type= "il1", inputArr=X)

        y = idft(X)

        print (f'il1 validity check {(abs(np.array(x))-abs(np.array(y)))/abs(np.array(x)) < 0.01}')
        plotTime(y, time)


        #print(map(X, lambda i: i/N))

        # compute the spectrum with your own DFT
        # you can use cmath.exp() for complex exponentials
        # plotSpectrum(x, Fs, type = 'your DFT name')

        # confirm DFT/IDFT correctness by checking if x == IDFT(DFT(x))

        # for further details, if any, check the lab document

    elif (sys.argv[1] == 'il2'):

        print('In-lab experiment 2 for the Fourier transform')

        # use np.random.randn() for randomization
        randArr = 10*np.random.randn(1000)
        time = np.array(list(range(1000)))
        #we can owverwrie the default values
        #frequency =  8.0                     # frequency of the signal
        #amplitude =  3.0                     # amplitude of the signal
        #phase = 1.0                          # phase of the signal
        #time, x = generateSin(Fs, interval, frequency, amplitude, phase)

        noiseDFT = dft(randArr)
        noiseIDFT = idft(noiseDFT)

        print(np.sum(np.square(abs(np.array(noiseDFT)))))
        print(np.sum(np.square(abs(np.array(noiseIDFT))))/1000)



        # You should also numerically check if the signal energy
        # in time and frequency domains is identical

        # for further details, if any, check the lab document

    elif (sys.argv[1] == 'il3'):

        print('In-lab experiment 3 for the Fourier transform')

        # generate randomized multi-tone signals
        # plot them in both time and frequency domain

        time, x = genMultiTone(Fs, interval)

        plotTime(x, time)
        # plot the signal in frequency domain
        plotSpectrum(x, Fs, type = 'FFT')

        # for further details, if any, check the lab document

    elif (sys.argv[1] == 'th'):

        print('Take-home exercise for the Fourier transform')
        #Fs, interval, frequency = 7.0, amplitude = 5.0, dutyCycle = 0.5
        time, x = generateSquare(Fs, interval, frequency = 10.0, amplitude = 0.5, dutyCycle = 0.5)

        plotTime(x, time)
            # plot the signal in frequency domain
        plotSpectrum(x, Fs, type = 'FFT')
        # for specific details check the lab document

    else:

        cli_error_msg()

    plt.show()
