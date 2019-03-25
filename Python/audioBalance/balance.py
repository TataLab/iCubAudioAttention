import numpy as np
import numpy.random as npr
import matplotlib.pyplot as plt
import librosa
import soundfile as sf

import os
import time

import argparse

def get_args():
    parser = argparse.ArgumentParser(description='balance')
    parser.add_argument('-r', '--root',     default='CODE',                            help='Environmental variable to datas root.                 (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',     default='data/audio_source',               help='Path from root of the data.                           (default: {})'.format('data/audio_source'))
    parser.add_argument('-t', '--target',   default='noise_samples/pink_noise_01.wav', help='Source of the audio to scale to.                      (default: {})'.format('noise_samples/pink_noise_01.wav'))
    parser.add_argument('-s', '--source',   default='target_samples',                  help='Directory of files to scale.                          (default: {})'.format('target_samples'))
    parser.add_argument('-S', '--save',     default='output/',                          help='Name of the local directory files will be saved in.   (default: {})'.format('output/'))
    parser.add_argument('-R', '--resample', default=-1, type=int,                      help='Resample the audio.                                   (default: {})'.format(-1))
    args = parser.parse_args()
    return args

def rms(data):
    return np.sqrt(np.mean(data ** 2))

args = get_args() 

# Check Paths.
root = os.environ.get(args.root)
if root == None:
    root = args.root
root = os.path.join(root, args.data)

source_dir  = os.path.join(root, args.source)
target_file = os.path.join(root, args.target)

if not os.path.exists(args.save):
    os.makedirs(args.save)

if not os.path.exists(source_dir):
    print("Path to Source Directory does not exist! [{}]".format(source_dir))
    exit()

if not os.path.exists(target_file):
    print("Target file to balance audio from does not exist! [{}]".format(target_file))
    exit()

print("Pulling Source data from     : {}".format(source_dir))
print("Scaling source to be same as : {}".format(target_file))

# Open the target file.
balance_data, balance_sampling_rate = sf.read(target_file)
balance_rms = rms(balance_data)

print("\nBalancing Audio to have RMS of {:.6f}".format(balance_rms))

# Walk through the source directory and open each file.
for root, dirs, files in os.walk(source_dir):

    # Create the subdirs that are found for output.
    for sub_dir in dirs:
        current_sub_dur = os.path.join(args.save, sub_dir)
        if not os.path.exists(current_sub_dur):
            os.makedirs(current_sub_dur)
            print("Created {}".format(current_sub_dur))
    
    print("Directory [{}]".format(root))

    # Open the files in the current directory and scale them.
    for file in files:
        current_file = os.path.join(root, file)
        output_file  = current_file.replace(source_dir, args.save)
   
        source, source_rate = sf.read(current_file)

        resampled = False
        if args.resample != -1 and args.resample != source_rate:
            resampled = True
            source = librosa.resample(source, source_rate, args.resample)
            source_rate = args.resample

        source_rms = rms(source)

        source *= ( balance_rms / source_rms )

        print("{:40s} : old={:.6f} | new={:.6f} \t\t {}".format(file, source_rms, rms(source), "Resampled" if resampled else " "))

        sf.write(output_file, source, source_rate)

    print("\n")


