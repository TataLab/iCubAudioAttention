import numpy as np
from scipy import interpolate
from scipy.signal import hilbert
import matplotlib.pyplot as plt
import pyaudio
import yarp

import argparse

def get_args():
    parser = argparse.ArgumentParser(description='filter')
    parser.add_argument('--rate',  default=48000, type=int,  help='Sampling rate                        (default: {})'.format(48000))
    parser.add_argument('--nchan', default=2,     type=int,  help='Number of Audio Channels in Array.   (default: {})'.format(2))
    parser.add_argument()
    args = parser.parse_args()
    return args


def main():
    
    # Init Yarp.
    yarp.Network.init()
    
    # Get parameters from a user.
    args = get_args()

    # Parse.
    samplingRate = args.rate
    numChannels  = args.nspeaker
    
    # For paFloat32 sample values must be in range [-1.0, 1.0]
    audioPlayer = pyaudio.PyAudio()
    audioStream = audioPlayer.open (
        format=pyaudio.paFloat32,
        channels=numChannels,
        rate=samplingRate,
        output=True
    )






    # Clean up.
    audioStream.stop_stream()
    audioStream.close()
    audioPlayer.terminate()
        



if __name__ == '__main__':
    main()