import numpy as np
import soundfile as sf

import os
import time

import argparse

def get_args():
    parser = argparse.ArgumentParser(description='s2w')
    parser.add_argument('-R', '--root',     default='CODE',                           help='Environmental variable to datas root.                 (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',     default='data/audio_experiment_{}_16384', help='Path from root of the data.                           (default: {})'.format('data/audio_experiment_{}_16384'))
    parser.add_argument('-e', '--exp',      default='01a',                            help='Which experiment to fill in the above ``{}``.         (default: {})'.format('{}', '01a'))
    parser.add_argument('-f', '--filename', default='yarpSound_{:04}',                help='Common Prefix for filenames.                          (default: {})'.format('yarpSound_{:04}'))
    parser.add_argument('-t', '--trial',    default=1,                      type=int, help='Source of the audio to scale to.                      (default: {})'.format(1))
    parser.add_argument('-s', '--save',     default='output.wav',                     help='Name of the local directory files will be saved in.   (default: {})'.format('output.wav'))
    parser.add_argument('-r', '--rate',     default=48000,                  type=int, help='Sampling rate of the audio.                           (default: {})'.format(48000))
    args = parser.parse_args()
    return args

args = get_args() 

# Check Paths.
root = os.environ.get(args.root)
if root == None:
    root = args.root
root = os.path.join(root, args.data.format(args.exp))

if not os.path.exists(root):
    print("Path ``{}`` does not exist!!".format(root))
    exit()

files = sorted(list(os.walk(root))[0][2])

source = args.filename.format(args.trial)
print("Looking for {}".format(source))

loadfile = ""
for file in files:
    if file.startswith(source):
        print("Found File named: {}".format(file))
        loadfile = file
        break

loadfile = os.path.join(root, loadfile)
print("Reading {}".format(loadfile))
data = np.loadtxt(loadfile, dtype=np.int16)

print("Saving File {}".format(args.save))
sf.write(args.save, data.T, args.rate)
