#
# Comp Eng 3DY4 (Computer Systems Integration Project)
#
# Copyright by Nicola Nicolici
# Department of Electrical and Computer Engineering
# McMaster University
# Ontario, Canada
#

import numpy as np
import math, cmath
from scipy import signal
import matplotlib.pyplot as plt
#
# you should use the demodulator based on arctan given below as a reference
#
# in order to implement your OWN FM demodulator without the arctan function,
# a very good and to-the-point description is given by Richard Lyons at:
#
# https://www.embedded.com/dsp-tricks-frequency-demodulation-algorithms/
#
# the demodulator boils down to implementing equation (13-117) from above, where
# the derivatives are nothing else but differences between consecutive samples
#
# needless to say, you should not jump directly to equation (13-117)
# rather try first to understand the entire thought process based on calculus
# identities, like derivative of the arctan function or derivatives of ratios
#

#
# use the four quadrant arctan function for phase detect between a pair of
# IQ samples; then unwrap the phase and take its derivative to demodulate
#

def impulseResponseBPF(Fs, pass_band, num_taps):
	normcenter = ((pass_band[1]+pass_band[0])/2)/(Fs/2)
	normpass = (pass_band[1]-pass_band[0])/(Fs/2)

	h = np.zeros(num_taps)

	for i in range(num_taps):
		if i == (num_taps-1)/2:
			h[i] = normpass
		else:
			h[i] = normpass*((math.sin(math.pi*(normpass/2.0)*(i-(num_taps-1)/2.0)))/(math.pi*(normpass/2.0)*(i-(num_taps-1)/2.0)))
		h[i] = h[i]*math.cos(i*math.pi*normcenter)
		h[i] = h[i]*math.sin((i*math.pi)/(num_taps))*math.sin((i*math.pi)/(num_taps))

	return h

def impulseResponseLPF(Fc, Fs, N):
	h = np.zeros(N)
	fc_norm = Fc/(Fs/2)

	for i in range(N):
		if i == (N-1)/2:
			h[i] = fc_norm
		else:
			h[i] = fc_norm*math.sin(math.pi*fc_norm*(i-(N-1)/2))/(math.pi*fc_norm*(i-(N-1)/2))
		h[i] = h[i]*math.sin(i*math.pi/N)*math.sin(i*math.pi/N)
	return h

def impulseResponseLPFupsampled(Fc, Fs, N, gain):
	h = np.zeros(N)
	fc_norm = Fc/(Fs/2)

	print(gain)
	for i in range(N):
		if i == (N-1)/2:
			h[i] = gain*fc_norm
		else:
			h[i] = gain*fc_norm*math.sin(math.pi*fc_norm*(i-(N-1)/2))/(math.pi*fc_norm*(i-(N-1)/2))
		h[i] = h[i]*math.sin(i*math.pi/N)*math.sin(i*math.pi/N)
	return h



def convfilter(filter_coeff, input_data, initial_state):
	n = len(input_data)
	k = len(filter_coeff)
	y = np.zeros(n)

	next_state = input_data[-k+1:]

	for i in range(len(y)):
		for m in range(k):
			if (i-m) < 0:
				y[i] += filter_coeff[m]*initial_state[i-m]
			elif (i-m >= n):
				pass
			else:
				y[i] += filter_coeff[m]*input_data[i-m]
	return y, next_state

def convfilter_resample(filter_coeff, input_data, initial_state, downsample, upsample, gain):
	n = len(input_data)
	k = len(filter_coeff)
	y = np.zeros(int(n*upsample/downsample))

	phase = 0

	next_state = input_data[-k+1:]
	

	for i in range(int(n*upsample/downsample)):
		phase = (i*downsample) % upsample
		for w in range(phase, k, upsample):
			x_index = int((i*downsample-w)/upsample)
			if (x_index < 0):
				y[i] += ((filter_coeff[w]*initial_state[x_index]))
			else:
				y[i] += (filter_coeff[w]*input_data[x_index])

	return y, next_state

def fmDemodArctan(I, Q, prev_phase = 0.0):
#
# the default prev_phase phase is assumed to be zero, however
# take note in block processing it must be explicitly controlled

	# empty vector to store the demodulated samples
	fm_demod = np.empty(len(I))

	# iterate through each of the I and Q pairs
	for k in range(len(I)):

		# use the atan2 function (four quadrant version) to detect angle between
		# the imaginary part (quadrature Q) and the real part (in-phase I)
		current_phase = math.atan2(Q[k], I[k])

		# we need to unwrap the angle obtained in radians through arctan2
		# to deal with the case when the change between consecutive angles
		# is greater than Pi radians (unwrap brings it back between -Pi to Pi)
		[prev_phase, current_phase] = np.unwrap([prev_phase, current_phase])

		# take the derivative of the phase
		fm_demod[k] = current_phase - prev_phase

		# save the state of the current phase
		# to compute the next derivative
		prev_phase = current_phase

	# return both the demodulated samples as well as the last phase
	# (the last phase is needed to enable continuity for block processing)
	return fm_demod, prev_phase

# custom function for DFT that can be used by the PSD estimate
def DFT(x):

	# number of samples
	N = len(x)

	# frequency bins
	Xf = np.zeros(N, dtype='complex')

	# iterate through all frequency bins/samples
	for m in range(N):
		for k in range(N):
			Xf[m] += x[k] * cmath.exp(1j * 2 * math.pi * ((-k) * m) / N)

	# return the vector that holds the frequency bins
	return Xf

def fmDemod(I, Q, i_prev = 0.0, q_prev = 0.0):
	fm_demod = np.empty(len(I))

	for k in range(len(I)):

		i_diff = I[k] - i_prev
		q_diff = Q[k] - q_prev

		i_prev = I[k]
		q_prev = Q[k]

		num = I[k]*q_diff - Q[k]*i_diff
		denom = I[k]**2 + Q[k]**2

		if denom == 0.0:
			fm_demod[k] = 0.0 #numpy throws warnings for x/0 and 0/0
		else:
			fm_demod[k] = num/denom

	return fm_demod, i_prev, q_prev

def freqzPlot(coeff, Fs, msg):

	# find the frequency response using freqz from SciPy:
	# https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.freqz.html
	w, h = signal.freqz(coeff)

	# Reminder: np.pi rad/sample is actually the Nyquist frequency
	w = w * Fs/(2*np.pi) # needed to draw the frequency on the X axis

	# plots the magnitude response where the x axis is normalized in rad/sample
	fig, ax1 = plt.subplots()
	ax1.set_title('Digital filter frequency response (' + msg + ')')
	ax1.plot(w, 20 * np.log10(abs(h)), 'b')
	ax1.set_ylabel('Amplitude [dB]', color='b')
	ax1.set_xlabel('Frequency [Hz]')

	# uncomment the lines below if you wish to inspect the phase response
	# Note: as important as the phase response is for some applications,
	# it is not critical at this stage because we expect a linear phase in the passband

	# ax2 = ax1.twinx()
	# angles = np.unwrap(np.angle(h))
	# ax2.plot(w, angles, 'g')
	# ax2.set_ylabel('Angle (radians)', color='g')

# custom function to estimate PSD based on the Bartlett method
# this is less accurate than the Welch method from matplotlib
# however, as the visual inspections confirm, the estimate gives
# the user a "reasonably good" view of the power spectrum
def estimatePSD(samples, NFFT, Fs):

	# rename the NFFT argument (notation consistent with matplotlib.psd)
	# to freq_bins (i.e., frequency bins for which we compute the spectrum)
	freq_bins = NFFT
	# frequency increment (or resolution of the frequency bins)
	df = Fs/freq_bins

	# create the frequency vector to be used on the X axis
	# for plotting the PSD on the Y axis (only positive freq)
	freq = np.arange(0, Fs/2, df)

	# design the Hann window used to smoothen the discrete data in order
	# to reduce the spectral leakage after the Fourier transform
	hann = np.empty(freq_bins)
	for i in range(len(hann)):
		hann[i] = pow(math.sin(i*math.pi/freq_bins),2)

	# create an empty list where the PSD for each segment is computed
	psd_list = []

	# samples should be a multiple of frequency bins, so
	# the number of segments used for estimation is an integer
	# note: for this to work you must provide an argument for the
	# number of frequency bins not greater than the number of samples!
	no_segments = int(math.floor(len(samples)/float(freq_bins)))

	# iterate through all the segments
	for k in range(no_segments):

		# apply the hann window (using pointwise multiplication)
		# before computing the Fourier transform on a segment
		windowed_samples = samples[k*freq_bins:(k+1)*freq_bins] * hann

		# compute the Fourier transform using the built-in FFT from numpy
		Xf = np.fft.fft(windowed_samples, freq_bins)

		# note, you can check how MUCH slower is DFT vs FFT by replacing the
		# above function call with the one that is commented below
		#
		# Xf = DFT(windowed_samples)
		#
		# note: the slow impelementation of the Fourier transform is not as
		# critical when computing a static power spectra when troubleshooting
		#
		# note also: time permitting a custom FFT can be implemented

		# since input is real, we keep only the positive half of the spectrum
		# however, we will also add the signal energy of negative frequencies
		# to have a better a more accurate PSD estimate when plotting
		Xf = Xf[0:int(freq_bins/2)] # keep only positive freq bins
		psd_seg = (1/(Fs*freq_bins/2)) * (abs(Xf)**2) # compute signal power
		psd_seg = 2*psd_seg # add the energy from the negative freq bins

		# translate to the decibel (dB) scale
		for i in range(len(psd_seg)):
			psd_seg[i] = 10*math.log10(psd_seg[i])

		# append to the list where PSD for each segment is stored
		# in sequential order (first segment, followed by the second one, ...)
		psd_list.extend(psd_seg)

	# compute the estimate to be returned by the function through averaging
	psd_est = np.zeros(int(freq_bins/2))

	# iterate through all the frequency bins (positive freq only)
	# from all segments and average them (one bin at a time ...)
	for k in range(int(freq_bins/2)):
		# iterate through all the segments
		for l in range(no_segments):
			psd_est[k] += psd_list[k + l*int(freq_bins/2)]
		# compute the estimate for each bin
		psd_est[k] = psd_est[k] / no_segments

	# the frequency vector and PSD estimate
	return freq, psd_est

# custom function to format the plotting of the PSD
def fmPlotPSD(ax, samples, Fs, height, title):

	x_major_interval = (Fs/12)		# adjust grid lines as needed
	x_minor_interval = (Fs/12)/4
	y_major_interval = 20
	x_epsilon = 1e-3
	x_max = x_epsilon + Fs/2		# adjust x/y range as needed
	x_min = 0
	y_max = 10
	y_min = y_max-100*height
	ax.psd(samples, NFFT=512, Fs=Fs)
	#
	# below is the custom PSD estimate, which is based on the Bartlett method
	# it less accurate than the PSD from matplotlib, however it is sufficient
	# to help us visualize the power spectra on the acquired/filtered data
	#
	# freq, my_psd = estimatePSD(samples, NFFT=512, Fs=Fs)
	# ax.plot(freq, my_psd)
	#
	ax.set_xlim([x_min, x_max])
	ax.set_ylim([y_min, y_max])
	ax.set_xticks(np.arange(x_min, x_max, x_major_interval))
	ax.set_xticks(np.arange(x_min, x_max, x_minor_interval), minor=True)
	ax.set_yticks(np.arange(y_min, y_max, y_major_interval))
	ax.grid(which='major', alpha=0.75)
	ax.grid(which='minor', alpha=0.25)
	ax.set_xlabel('Frequency (kHz)')
	ax.set_ylabel('PSD (db/Hz)')
	ax.set_title(title)

if __name__ == "__main__":

	# do nothing when this module is launched on its own
	pass
