import numpy as np
import numpy.random as npr
import matplotlib.pyplot as plt
from scipy.special import comb

import csv
import itertools
import os
import time

import argparse

def get_args():
    parser = argparse.ArgumentParser(description='player')
    parser.add_argument('-r', '--root',   default='CODE',                        help='Environmental variable to datas root.       (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',   default='data/audio_source',           help='Which data folder to stream from.           (default: {})'.format('data/audio_source'))
    parser.add_argument('-t', '--target', default='target_samples',              help='Target samples directory.                   (default: {})'.format('target_samples'))
    parser.add_argument('-b', '--balan',  default=True, action='store_false',    help='Balance the number of set sizes.            (default: {})'.format(True))
    parser.add_argument('-s', '--save',   default='experiment_04.csv',           help='Folder to save processed matrix to.         (default: {})'.format('experiment_04.csv'))
    parser.add_argument('-c', '--chan',   default=2,   type=int,                 help='Number of Channels to expect.               (default: {})'.format(2))
    parser.add_argument('-x', '--seed',   default=123, type=int,                 help='Seed for Numpy RNG.                         (default: {})'.format(123))
    args = parser.parse_args()
    return args

args = get_args()

data_dir = os.environ.get(args.root)
if data_dir == None:    data_dir = args.root
data_dir = os.path.join(data_dir,  args.data)

target_dir = os.path.join(data_dir, args.target)

print("{:12s} : {}  {}".format("Target Path", "\u2713" if os.path.exists(target_dir)  else "x", target_dir))

if not os.path.exists(target_dir):
    print("Could not find Path . . .")
    exit()

# Find the number of trials per set size.
trials_per_set_size = []
for idx in range(args.chan):
    trials_per_set_size.append( comb(args.chan, 1, exact=True) * comb(args.chan-1, idx, exact=True) )

# Balance the number of trials.
if args.balan:
    trials_per_set_size = [ np.max(trials_per_set_size) ] * args.chan

# Keep a running tally.
trials_tally = [ 0 ] * args.chan
print("Set Sizes to Make : {}".format(trials_per_set_size))

# Seed the RNG.
np.random.seed(args.seed)

# Pull the path to the targets.
target_files = list(os.walk(target_dir))[0][2]
numTarget    = len(target_files)
useTarget    = np.arange(numTarget)


# Write the output to a csv.
with open(args.save, 'w') as csvfile:
    writer = csv.writer(csvfile, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

    header = [ 'Trial', 'Target' ] + [ str(idx) for idx in range(args.chan) ]
    writer.writerow(header)

    rowCount = 1
    speaker_set = [ idx for idx in range(args.chan) ]

    for size in range(1, args.chan+1):
        
        # Don't make too many trials for one size.
        while trials_tally[size-1] < trials_per_set_size[size-1]:
            
            subsets = np.asarray( list(itertools.combinations(speaker_set, size)) )
            npr.shuffle(subsets)

            for subset in subsets:

                # Don't make too many trials for one size.
                if trials_tally[size-1] >= trials_per_set_size[size-1]:
                    break

                # Shuffle for 'fairness'.
                npr.shuffle(subset)

                # Take turns for which idx is the 'target'.
                for trg in range(size):

                    # Don't make too many trials for one size.
                    if trials_tally[size-1] >= trials_per_set_size[size-1]:
                        break

                    # Make an empty row.
                    speakers = [ 'NONE' ] * args.chan

                    # Ensure Unique Audio Sources.
                    npr.shuffle( useTarget )

                    # Set all noise distractors.
                    for idx in range(size):
                        speakers[subset[idx]] = os.path.join(target_dir, target_files[ useTarget[idx] ])
                        #speakers[subset[idx]] = target_files[ useTarget[idx+1] ]
                        
                    # Replace one of the distractors with the true target.
                    #speakers[subset[trg]] = os.path.join(target_dir, target_files[ useTarget[0] ])
                    #speakers[subset[trg]] = target_files[ useTarget[0] ]
                    
                    # Turn this into a row for the csv.
                    row = [ str(rowCount), ''.join(map(str, subset)) ] + speakers
                    writer.writerow(row)

                    rowCount += 1
                    trials_tally[ size-1 ] += 1

print("Set Sizes Made    : {}".format(trials_tally))
print("Generated {} Trials.".format(rowCount-1))
