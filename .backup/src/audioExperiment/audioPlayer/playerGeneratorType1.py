import numpy as np
import matplotlib.pyplot as plt

import csv
import os
import time

import argparse

def get_args():
    parser = argparse.ArgumentParser(description='player')
    parser.add_argument('-r', '--root',  default='CODE',                             help='Environmental variable to datas root.      (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',  default='data/audio_source/target_samples', help='Which data folder to stream from.          (default: {})'.format('data/audio_source/target_samples'))
    parser.add_argument('-s', '--save',  default='experiment_01.csv',                help='Folder to save processed matrix to.        (default: {})'.format('experiment_01.csv'))
    parser.add_argument('-c', '--chan',  default=2, type=int,                        help='Number of Channels to expect.              (default: {})'.format(2))
    
    args = parser.parse_args()
    return args



args = get_args()

data_dir = os.environ.get(args.root)
if data_dir == None:
    data_dir =  args.root
data_dir = os.path.join(data_dir, args.data)

print("Building Trial Sheet from files in {}".format(data_dir))

if not os.path.exists(data_dir):
    print("Could not find Directory ``{}``. \n Please Provide a path containing audio data.".format(data_dir))
    exit()

rowCount = 1

with open(args.save, 'w') as csvfile:
    writer = csv.writer(csvfile, delimiter=',', quotechar='|', quoting=csv.QUOTE_MINIMAL)

    header = [ 'Trial', 'Target' ] + [ str(idx) for idx in range(args.chan) ]
    writer.writerow(header)

    for root, _, files in os.walk(data_dir):
        for name in sorted(files):

            if (not name.endswith('.flac')) and (not name.endswith('.wav')):
                continue

            source = os.path.join(root, name)            

            for idx in range(args.chan):

                speakers = ['NONE'] * args.chan
                speakers[idx] = source

                row = [str(rowCount), str(idx)] + speakers

                writer.writerow(row)

                rowCount += 1

print("Generated {} Trials.".format(rowCount-1))

