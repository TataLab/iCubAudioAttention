import numpy as np
import matplotlib.pyplot as plt

import os
import time
import yarp

import argparse

# Init Yarp.
yarp.Network.init()

def get_args():
    parser = argparse.ArgumentParser(description='runner')
    parser.add_argument('-n', '--name',  default='/audioRunner',           help='Name for the module.                       (default: {})'.format('/audioRunner'))
    parser.add_argument('-r', '--root',  default='CODE',                   help='Environmental variable to datas root.      (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',  default='audio_experiment_16384', help='Which data folder to stream from.          (default: {})'.format('audio_experiment_16384'))
    parser.add_argument('-s', '--save',  default='env',                    help='Folder to save processed matrix to.        (default: {})'.format('env_16384'))
    parser.add_argument('-f', '--frame', default=16384, type=int,          help='Length of frames to stream.                (default: {})'.format(16384))
    parser.add_argument('-R', '--rate',  default=48000, type=int,          help='Sampling rate audio was recorded at        (default: {})'.format(48000))
    args = parser.parse_args()
    return args


class audioRunner(object):

    def __init__(self, args):

        # Get all vars from args.
        self.port_name    = args.name
        self.data_dir     = os.environ.get(args.root) + '/data/'
        self.data         = args.data
        self.save_name    = args.save
        self.frameLength  = args.frame
        self.samplingRate = args.rate

        self.source_dir = self.data_dir + self.data
        self.target_dir = self.data_dir + "processed/" + self.save_name + "_" + str(self.frameLength)

        if not os.path.exists(self.target_dir):
            os.makedirs(self.target_dir)

        # Send audio to the processor.
        self.audio_out_port = yarp.Port()
        self.audio_out_port.open(self.port_name + "/rawAudio:o")

        # For reading in the Bayesian map.
        self.matrix_in_port = yarp.Port()
        self.matrix_in_port.open(self.port_name + "/matrix:i")

        # For flushing the Bayesian Map.
        self.bayes_out_port = yarp.Port()
        self.bayes_out_port.open(self.port_name + "/bayesClear:o")


    def run(self):

        while True:

            if self.audio_out_port.getOutputCount() and self.matrix_in_port.getInputCount() and self.bayes_out_port.getOutputCount():
                self.processing()
                print("\n\nAll Data Processed. Good Bye!\n")
                exit()
            
            else:
                msg = "Missing Connection to "
                if not self.audio_out_port.getOutputCount(): msg += "audio output; "
                if not self.matrix_in_port.getInputCount():  msg += "matrix input; "
                if not self.bayes_out_port.getOutputCount(): msg += "bayes clear; "
                msg += ". . ."
                print(msg)
                time.sleep(1)
        

    def processing(self):

        count = 1

        for root, _, files in os.walk(self.source_dir):
            for name in sorted(files):
                
                if not name.endswith('.data'):
                    continue

                source = os.path.join(root, name)

                target_name = name.replace("yarpSound_", "_")
                target_name = self.save_name + target_name.replace(".data", ".npy")
                target = os.path.join(self.target_dir, target_name)

                print("Processing {:03d} : {}  ==>  {}".format(count, name, target_name), end="   ", flush=True)
                
                # Break into left and right channel for easier slicing.
                RawData   = np.loadtxt(source, dtype=np.int)
                L_RawData = RawData[0].reshape(-1, self.frameLength)
                R_RawData = RawData[1].reshape(-1, self.frameLength)
                numFrames = L_RawData.shape[0]
                
                # Keep time.
                timeStamp = yarp.Stamp()
                timeStamp.update()

                # Clear the Bayesian Map.
                command = yarp.Bottle()
                command.clear()
                command.addString("clear")
                self.bayes_out_port.write(command)

                # Buffer for returned matrices.
                MatrixBuffer = []

                startTime = time.time()

                # Send each frame and get a response.
                for idx in range(numFrames):

                    # Convert to yarp sound object.
                    sound = self._sound_process(L_RawData[idx], R_RawData[idx])

                    # Publish the audio on the network.
                    self.audio_out_port.setEnvelope(timeStamp)
                    self.audio_out_port.write(sound)

                    # Wait for a response.
                    yarp_matrix = yarp.Matrix()
                    self.matrix_in_port.read(yarp_matrix) 

                    # Append this matrix to the buffer.
                    MatrixBuffer.append(self._matrix_process(yarp_matrix))

                    # Update the time stamp.
                    timeStamp.update()

                # Save the matrix out.
                np.save(target, np.asarray(MatrixBuffer))

                count  += 1
                endTime = time.time()
                print("\u0394 : {:.4f}".format(endTime-startTime))


    def _sound_process(self, Left_Ch, Right_Ch):

        # Set some params.
        numChan = 2
        numSamp = Left_Ch.shape[0]

        yarp_sound = yarp.Sound()
        yarp_sound.resize(numSamp, numChan)
        yarp_sound.setFrequency(self.samplingRate)

        for idx in range(numSamp):
            yarp_sound.set(int(Left_Ch[idx]), int(idx), 0)
            yarp_sound.set(int(Right_Ch[idx]), int(idx), 1)

        return yarp_sound

    
    def _matrix_process(self, yarp_matrix):
        
        # Get the size of the original matrix.
        numRows = yarp_matrix.rows()
        numCols = yarp_matrix.cols()

        # Convert to a string, and convert to numpy matrix.
        string_mat = yarp_matrix.toString(10, 1)    # (precision, tabs:0 space:1)
        np_matrix = np.fromstring(string_mat, sep=' ').reshape((numRows,numCols))
        return np_matrix

        
    def cleanup(self):
        print("Closing YARP Ports.")
        self.audio_out_port.close()
        self.matrix_in_port.close()


def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the audio runner module.
    runner = audioRunner(args)

    # Begin running the module.
    try:
        runner.run()

    finally:
        runner.cleanup()
        

if __name__ == '__main__':
    main()