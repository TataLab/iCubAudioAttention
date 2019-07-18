import argparse
import numpy as np
import pyaudio
import wave

def rms(data):
    return np.sqrt(np.mean(data ** 2))

# Grab some parameters from the user.
parser = argparse.ArgumentParser(description='Pure Tone (Sine) Generator')
parser.add_argument( '-d', '--dur',      default=2.0,       type=float,      help='Duration (in seconds) that the pure tone should be generated. (default: {})'.format(2.0))
parser.add_argument( '-r', '--rate',     default=48000,     type=int,        help='Sampling rate (in Hz) for the pure tone. (default: {})'.format(48000))
parser.add_argument( '-c', '--chan',     default=10,        type=int,        help='Number of channels. (default: {})'.format(10))
parser.add_argument( '-p', '--play',     default=5,         type=int,        help='Play at location. (default: {})'.format(5))
parser.add_argument( '-f', '--find',     default=False, action='store_true', help='Continuously Print RMS of recorded. (default: {})'.format(False))
parser.add_argument( '-b', '--balance',  default=0.045011,  type=float,      help='Balance the RMS to. (default: {})'.format(0.045011))
parser.add_argument( '-w', '--width',    default=0.003,     type=float,      help='Width of threshold. (default: {})'.format(0.003))
args = parser.parse_args()


# For paFloat32 sample values must be in range [-1.0, 1.0]
p = pyaudio.PyAudio()
stream = p.open(
    format=pyaudio.paFloat32,
    channels=args.chan,
    rate=args.rate,
    input=True,
    frames_per_buffer=args.rate
)

try:
    while True:

        # Read in one second of audio.
        buffer = stream.read(args.rate)
        data = np.frombuffer(buffer, dtype=np.float32)

        # Find the RMS of the data.
        data_rms = rms(data)

        if args.find:
            print("{:.4f}".format(data_rms))
        else:
            print( "{:.4f} | {:.4f} --> {}".format (
                    data_rms, 
                    args.balance,
                    "HIGH" if data_rms > args.balance + args.width else ("LOW" if data_rms < args.balance - args.width else "GLZ" )
                )
            )

finally:
    print("Killing the buffer.")
    stream.stop_stream()
    stream.close()
    p.terminate()
