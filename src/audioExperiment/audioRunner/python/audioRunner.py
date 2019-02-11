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
    parser.add_argument('-n', '--name',  default='/audioRunner',                     help='Name for the module.                                  (default: {})'.format('/audioRunner'))
    parser.add_argument('-r', '--root',  default='CODE',                             help='Environmental variable to datas root.                 (default: {})'.format('CODE'))
    parser.add_argument('-d', '--datap', default='data/audio_experiment_01_16384',   help='Which data folder to stream from.                     (default: {})'.format('data/audio_experiment_01_16384'))
    parser.add_argument('-s', '--savep', default='data/processed/ae_01_env_16384',   help='Folder to save processed matrix to.                   (default: {})'.format('data/processed/ae_01_env_16384'))
    parser.add_argument('-S', '--saven', default='bayes_env',                        help='Name of the files to be saved.                        (default: {})'.format('bayes_env'))
    parser.add_argument('-f', '--frame', default=16384, type=int,                    help='Length of frames to stream.                           (default: {})'.format(16384))
    parser.add_argument('-R', '--rate',  default=48000, type=int,                    help='Sampling rate audio was recorded at                   (default: {})'.format(48000))
    parser.add_argument('-m', '--move',  default=False, action='store_true',         help='Enable Movements to be sent to processing             (default: {})'.format(False))
    parser.add_argument('-p', '--play',  default=False, action='store_true',         help='Playback the audio instead for sending for processing (default: {})'.format(False))
    args = parser.parse_args()
    return args


class audioRunner(object):

    def __init__(self, args):

        # Get all vars from args.
        self.port_name     = args.name
        self.root_dir      = os.environ.get(args.root)
        self.data_path     = args.datap
        self.save_path     = args.savep
        self.save_name     = args.saven
        self.frame_length  = args.frame
        self.sampling_rate = args.rate
        self.movements     = args.move
        self.play_back     = args.play 

        self.source_dir = os.path.join(self.root_dir, self.data_path)
        self.target_dir = os.path.join(self.root_dir, self.save_path)

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

        # For Sending Head Positions.
        if self.movements:
            self.head_out_port = yarp.Port()
            self.head_out_port.open(self.port_name + "/headAngle:o")


    def run(self):

        while True:

            if self.play_back:
                yarp.Network.connect("/audioRunner/rawAudio:o", "/yarphear")
                self.playback()
                print("\n\nAll Data Streamed. Good Bye!\n")
                exit()

            elif self.audio_out_port.getOutputCount() and self.matrix_in_port.getInputCount() and self.bayes_out_port.getOutputCount() and ((not self.movements) or self.head_out_port.getOutputCount()):
                self.processing()
                print("\n\nAll Data Processed. Good Bye!\n")
                exit()
            
            else:
                msg = "Missing Connection to "
                if not self.audio_out_port.getOutputCount(): msg += "audio output; "
                if not self.matrix_in_port.getInputCount():  msg += "matrix input; "
                if not self.bayes_out_port.getOutputCount(): msg += "bayes clear; "
                if self.movements and not self.head_out_port.getOutputCount:
                    msg += "head angle; "

                msg += ". . ."
                print(msg)
                time.sleep(1)
        


    def processing(self):

        count = 1

        root  = list(os.walk(self.source_dir))[0][0]
        files = sorted(list(os.walk(self.source_dir))[0][2])

        files_sound = []
        [ files[idx].startswith('yarpSound') and files_sound.append(files[idx]) for idx in range(len(files)) ]
        files_sound = sorted(files_sound)

        files_pos = []
        [ files[idx].startswith('yarpPan') and files_pos.append(files[idx]) for idx in range(len(files)) ]
        files_pos = sorted(files_pos)

        num_files = len(files_sound)

        for trial in range(num_files):
            
            # Work out file names
            sound_name = files_sound[trial]
            sound_source = os.path.join(root, sound_name)

            if self.movements:
                position_name = files_pos[trial]
                position_source = os.path.join(root, position_name)

            target_name = sound_name.replace("yarpSound_", "_")
            target_name = self.save_name + target_name.replace(".data", ".npy")
            target = os.path.join(self.target_dir, target_name)            

            print("Processing {:04d} : {}  ==>  {}".format(count, sound_name, target_name), end="   ", flush=True)

            # Begin Reading in Data.
            RawData   = np.loadtxt(sound_source, dtype=np.int)
            L_RawData = RawData[0].reshape(-1, self.frame_length)
            R_RawData = RawData[1].reshape(-1, self.frame_length)
            numFrames = L_RawData.shape[0]

            if self.movements:
                # Read in the positions from the file.
                RawPos = np.loadtxt(position_source, dtype=np.float32).tolist()
                numPos = len(RawPos)
                scale  = ( numFrames // numPos )
                
                # Increase the number of position readings relative to the scale
                RawPos = RawPos * scale
                RawPos = np.asarray(RawPos, dtype=np.float32).reshape(-1,scale,order='F').reshape(-1,order='C')

                print("\n\n", RawPos.shape, L_RawData.shape, R_RawData.shape)


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

                # Publish the movements on the network.
                if self.movements:
                    headPos = yarp.Bottle()
                    headPos.clear()
                    headPos.addDouble(RawPos[idx])
                    self.head_out_port.write(headPos)

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



        """
        for name in sorted(files):
            
            if not name.endswith('.data'):
                continue

            source = os.path.join(root, name)

            target_name = name.replace("yarpSound_", "_")
            target_name = self.save_name + target_name.replace(".data", ".npy")
            target = os.path.join(self.target_dir, target_name)

            print("Processing {:04d} : {}  ==>  {}".format(count, name, target_name), end="   ", flush=True)
            
            # Break into left and right channel for easier slicing.
            RawData   = np.loadtxt(source, dtype=np.int)
            L_RawData = RawData[0].reshape(-1, self.frame_length)
            R_RawData = RawData[1].reshape(-1, self.frame_length)
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
        """



    def playback(self):

        root  = list(os.walk(self.source_dir))[0][0]
        files = sorted(list(os.walk(self.source_dir))[0][2])

        files_sound = []
        [ files[idx].startswith('yarpSound') and files_sound.append(files[idx]) for idx in range(len(files)) ]
        files = sorted(files_sound)

        print(files)

        while True:
            
            idx = int(input("\nPick a Trial in range [{}, {}] : ".format(1, len(files))))
            
            if idx < 1 or idx > len(files):
                print("Index {} out of range.".format(idx))
                continue

            source = files[idx-1]

            print("Streaming {:20s} ".format(source), end="   ", flush=True)

            source = os.path.join(root, source)
                
            # Break into left and right channel for easier slicing.
            RawData   = np.loadtxt(source, dtype=np.int)
            L_RawData = RawData[0].reshape(-1, self.frame_length)
            R_RawData = RawData[1].reshape(-1, self.frame_length)
            numFrames = L_RawData.shape[0]

            # Keep time.
            timeStamp = yarp.Stamp()
            timeStamp.update()
            startTime = time.time()

            # Send each frame and get a response.
            for idx in range(numFrames):

                # Convert to yarp sound object.
                sound = self._sound_process(L_RawData[idx], R_RawData[idx])

                # Publish the audio on the network.
                self.audio_out_port.setEnvelope(timeStamp)
                self.audio_out_port.write(sound)

                # Sleep for a bit for playing back.
                time.sleep( (self.frame_length / self.sampling_rate) - 0.08 )

                # Update the time stamp.
                timeStamp.update()
            
            endTime = time.time()
            print("\u0394 : {:.4f}".format(endTime-startTime))



    def _sound_process(self, Left_Ch, Right_Ch):

        # Set some params.
        numChan = 2
        numSamp = Left_Ch.shape[0]

        yarp_sound = yarp.Sound()
        yarp_sound.resize(numSamp, numChan)
        yarp_sound.setFrequency(self.sampling_rate)

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