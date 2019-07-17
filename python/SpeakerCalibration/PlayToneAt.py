import argparse
import numpy as np
import pyaudio
import wave


def rms(data):
    return np.sqrt(np.mean(data ** 2))


def genPureTone(freq, rate, sec, chan, balance):
    samples = np.zeros(int(sec * rate), dtype=np.float32 )
    samples = np.sin(2 * np.pi * np.arange(rate * sec) * freq / rate).astype(np.float32)

    samples_rms = rms(samples)
    samples    *= ( balance / samples_rms )
    post_rms    = rms(samples)

    return samples, samples_rms, post_rms


# Grab some parameters from the user.
parser = argparse.ArgumentParser(description='Pure Tone (Sine) Generator')
parser.add_argument( '-f', '--freq',    default=1000.0,    type=float,  help='Frequency of the tone to be generated (default: {})'.format(1000.0))
parser.add_argument( '-d', '--dur',     default=2.0,       type=float,  help='Duration (in seconds) that the pure tone should be generated. (default: {})'.format(2.0))
parser.add_argument( '-r', '--rate',    default=48000,     type=int,    help='Sampling rate (in Hz) for the pure tone. (default: {})'.format(48000))
parser.add_argument( '-c', '--chan',    default=10,        type=int,    help='Number of channels. (default: {})'.format(10))
parser.add_argument( '-p', '--play',    default=5,         type=int,    help='Play at location. (default: {})'.format(5))
parser.add_argument( '-b', '--balance', default=0.045011,  type=float,  help='Balance the RMS to. (default: {})'.format(0.045011))
args = parser.parse_args()


# For paFloat32 sample values must be in range [-1.0, 1.0]
p = pyaudio.PyAudio()
stream = p.open(
    format=pyaudio.paFloat32,
    channels=args.chan,
    rate=args.rate,
    output=True
)

# Get the pure tone.
samples = np.zeros( (args.chan, int(args.dur * args.rate)), dtype=np.float32 )

pure_tone, tone_rms, post_rms = genPureTone(args.freq, args.rate, args.dur, args.chan, args.balance)

if args.play == -1:
    samples[:] = pure_tone
else:
    samples[args.play] = pure_tone

print("Playing {} Hz Pure tone on Channel {}. OG-RMS:{} | NEW-RMS:{}".format(
    args.freq,
    args.play,
    tone_rms,
    post_rms
))

# Play the tone.
stream.write(samples.transpose().tobytes())

# Clean up.
stream.stop_stream()
stream.close()
p.terminate()