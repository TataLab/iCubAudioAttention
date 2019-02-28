import numpy as np
import numpy.random as npr
import matplotlib.pyplot as plt

import csv
import itertools
import os
import time

import argparse

def get_args():
    parser = argparse.ArgumentParser(description='runner')
    parser.add_argument('-r', '--root',   default='CODE',                        help='Environmental variable to datas root.       (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',   default='data/audio_source',           help='Which data folder to stream from.           (default: {})'.format('data/audio_source'))
    parser.add_argument('-t', '--target', default='target_samples',              help='Target samples directory /male and /female. (default: {})'.format('target_samples'))
    parser.add_argument('-n', '--noise',  default='noise_samples',               help='Noise samples directory.                    (default: {})'.format('noise_samples'))
    parser.add_argument('-s', '--save',   default='experiment_02.csv',           help='Folder to save processed matrix to.         (default: {})'.format('experiment_02.csv'))
    parser.add_argument('-c', '--chan',   default=2,   type=int,                 help='Number of Channels to expect.               (default: {})'.format(2))
    parser.add_argument('-x', '--seed',   default=123, type=int,                 help='Seed for Numpy RNG.                         (default: {})'.format(123))
    args = parser.parse_args()
    return args


args = get_args()

data_dir = os.environ.get(args.root)
if data_dir == None:
    data_dir =  args.root
data_dir = os.path.join(data_dir, args.data)

target_dir = os.path.join(data_dir, args.target)
noise_dir  = os.path.join(data_dir, args.noise)

print("Path to Target Directory : {}".format(target_dir))
print("Path to Noise Directory  : {}".format(noise_dir))

target_types = ["male", "female"]
check_list   = [ data_dir, target_dir, os.path.join(target_dir, "male"), os.path.join(target_dir, "female"), noise_dir]

for dir_check in check_list:
    if not os.path.exists(dir_check):
        print("\n\nCould not find Directory ``{}``. \n Please Provide a path containing audio data.".format(dir_check))
        exit()


# Seed the RNG.
np.random.seed(args.seed)

# Pull the path to the noise.
noise_files = list(os.walk(noise_dir))[0][2]
numNoise    = len(noise_files)
useNoise    = np.arange(numNoise)

# Write the output to a csv.
with open(args.save, 'w') as csvfile:
    writer = csv.writer(csvfile, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

    header = [ 'Trial', 'Target' ] + [ str(idx) for idx in range(args.chan) ]
    writer.writerow(header)
    
    rowCount = 1
    for channel in range(args.chan):
        for sub_dir in target_types:

            # Get all the files in the current directory.
            root  = os.path.join(target_dir, sub_dir)
            files = list(os.walk(root))[0][2]
            numFiles = len(files)

            # Create a set of speakers without the current channel.
            speaker_set = [ idx for idx in range(args.chan) ]
            speaker_set.pop(channel)

            # Generate all combinations of speakers that are not the target.
            for size in range(1, len(speaker_set)+1):
                for subset in itertools.combinations(speaker_set, size):

                    # Set the whole row to NONE for file.
                    speakers = ['NONE'] * args.chan

                    # Set the current speaker to a random file in our target dir.
                    speakers[channel] = os.path.join(root, files[ npr.randint(numFiles) ])

                    # Make sure we don't have duplicates.
                    npr.shuffle( useNoise )
                    for idx in range(size):
                        speakers[subset[idx]] = os.path.join(noise_dir, noise_files[ useNoise[idx] ])

                    # Turn this into a row for the CSV.
                    row = [str(rowCount), str(channel)] + speakers
                    writer.writerow(row)

                    rowCount += 1


print("Generated {} Trials.".format(rowCount-1))
