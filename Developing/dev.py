from scipy.io import wavfile
from scipy.signal import find_peaks, iirnotch, freqz, filtfilt
from scipy.fft import fft, fftfreq
import matplotlib.pyplot as plt
import numpy as np


def rta(frame, window, N):
    """ frame rta with window
        @ param: frame
        @ param: window: taper window to prevent spectral leakage
        @ param: N: sample count
        @ return: trasformed frame
    """
    frame = frame * window
    tranformed_frame = np.power(np.abs((fft(frame, N))), 2)
    return tranformed_frame


def pnpr(frame, thr):
    """ peak to neighbour power ratio:
        @ param: frame
        @ param: thr: detection threshold (suggested: 30)
        @ return: array of possible howling frequencies according to pnpr method
    """
    peaks = []
    """ c like implementation :
    i = 2
    while i < (len(frame)-2):
        if 10*np.log10(frame[i]/frame[i-2]) > thr :
            if 10*np.log10(frame[i]/frame[i-1]) > thr:
                if 10*np.log10(frame[i]/frame[i+1]) > thr:
                    if 10*np.log10(frame[i]/frame[i+1]) > thr:
                        peaks.append(i)
    return peaks
    """
    frame = 10*np.log10(frame)
    peaks, _ = find_peaks(frame, threshold=thr)
    return peaks


def phpr(peaks, frame, thr):
    """ peak to harmonics power ratio:
        @ param: peaks: possible peaks to be analyzed
        @ param: frame
        @ param: detection threshold (suggested: 10)
        @ return: array of possible howling frequencies according to phpr method
    """
    n_peaks = []
    len = frame.size
    for peak in peaks:
        if 10*np.log10(frame[peak]/frame[int(peak/2)]) > thr:
            if 2*peak < len:
                if 10*np.log10(frame[peak]/frame[2*peak]) > thr:
                    n_peaks.append(peak)
            else:
                n_peaks.append(peak)
    return n_peaks


def generate_filter(howlings, df, Q, fs):
    """ generate filter:
        @ param: howlings: found howling indexes
        @ param: df: frequency resolution
        @ param: Q: Quality factor, Q = w0/bw, w0 = center frequency, bw = -3dB bandwidth
        @ return: array of filters
    """
    iir_notch = []
    for howl in howlings:
        a, b = iirnotch(howl*df, Q, fs)
        iir_notch.append([a, b])
        # Frequency response
        freq, h = freqz(b, a, fs=fs)
        # Plot
        fig, ax = plt.subplots(2, 1, figsize=(8, 6))
        ax[0].plot(freq, 20*np.log10(abs(h)), color='blue')
        ax[0].set_title("Frequency Response")
        ax[0].set_ylabel("Amplitude (dB)", color='blue')
        ax[0].grid()
        ax[1].plot(freq, np.unwrap(np.angle(h))*180/np.pi, color='green')
        ax[1].set_ylabel("Angle (degrees)", color='green')
        ax[1].set_xlabel("Frequency (Hz)")
        ax[1].grid()
        #plt.show()
    return iir_notch

def apply_filter(frame,filters):
    for filter in filters:
        frame = filtfilt(filter[0],filter[1],frame)
    return frame


if __name__ == '__main__':
    testfile = '/home/andre/CLionProjects/AntiLarsen/Developing/Resources/test.wav'
    fs, samples = wavfile.read(testfile)
    # sample points
    N = 2048
    # time spacing
    T = 1 / fs
    # freq specing
    f_axis = fftfreq(N, T)[:N//2]
    df = f_axis[1]
    # blackman window
    blackman = np.blackman(N)
    # debug info
    print('Sampling rate: ' + str(fs) + " Hz")
    print('Buffer Length: ' + str(N) + " Samples, " + str(N*T*1000) + " ms")
    print('Frequency resolution: ' + str(df) + ' Hz')
    # right channel scomposition and fft
    frame_r = samples[0:N, 0]
    ft_frame_r = rta(frame_r, blackman, N)
    # consider only positive freq
    ft_frame_r = (ft_frame_r[0:N//2])
    # find peaks
    peaks = pnpr(ft_frame_r, 5)
    peaks = phpr(peaks, ft_frame_r, 1)
    filters = generate_filter(peaks, df, 30, fs)
    filtered_frame = apply_filter(frame_r,filters)
    
    print(filters)
    print(peaks)
    # check filter effectivness
    peaks = phpr(peaks, filtered_frame, 1)
    print(peaks)

    # debug plot
    #plt.plot(f_axis, ft_frame_r)
    #plt.plot(peaks * df, ft_frame_r[peaks], "x")
    # plt.grid()
    # plt.show()
