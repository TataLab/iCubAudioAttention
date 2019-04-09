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
    parser.add_argument('-n', '--name',    default='/audioRunner',                     help='Name for the module.                                       (default: {})'.format('/audioRunner'))
    parser.add_argument('-r', '--root',    default='CODE',                             help='Environmental variable to datas root.                      (default: {})'.format('CODE'))
    parser.add_argument('-d', '--data',    default='data/recorded/env_01b_2048',       help='Which data folder to stream from.                          (default: {})'.format('data/recorded/env_01b_2048'))
    parser.add_argument('-s', '--save',    default='data/processed/env_01b',           help='Folder to save processed matrix to. Frame Len is appended. (default: {})'.format('data/processed/env_01b'))
    parser.add_argument('-a', '--ampp',    default='Amp',                              help='Prefix for the allo-amplitude save name.                   (default: {})'.format('Amp'))
    parser.add_argument('-e', '--envp',    default='Env',                              help='Prefix for the allo-envelops save name.                    (default: {})'.format('Env'))
    parser.add_argument('-o', '--origin',  default=2048,  type=int,                    help='Original length of frames recorded at.                     (default: {})'.format(2048))
    parser.add_argument('-f', '--frame',   default=16384, type=int,                    help='Length of frames to stream.                                (default: {})'.format(16384))
    parser.add_argument('-R', '--rate',    default=48000, type=int,                    help='Sampling rate audio was recorded at.                       (default: {})'.format(48000))
    parser.add_argument('-m', '--move',    default=False, action='store_true',         help='Enable recorded movements to be sent to processing.        (default: {})'.format(False))
    parser.add_argument('-S', '--strat',   default=2,     type=int,                    help='Head movement sending strategy. [1, 2]                     (default: {})'.format(2))
    parser.add_argument('-O', '--over',    default=0,     type=int,                    help='Number of samples to overlap for strategy 2.               (default: {})'.format(0))
    parser.add_argument('-T', '--thresh',  default=0.5,   type=float,                  help='Threshold of head position differences.                    (default: {})'.format(0.5))
    parser.add_argument('-P', '--play',    default=False, action='store_true',         help='Playback the audio instead for sending for processing.     (default: {})'.format(False))
    parser.add_argument('-c', '--connect', default=False, action='store_true',         help='Automatically connect all ports on construction.           (default: {})'.format(False))
    args = parser.parse_args()

    strategies = [1, 2]
    if not args.strat in strategies:
        print("Unknown Strategy Number : {}".format(args.strat))
        exit()

    return args


class audioRunner(object):

    def __init__(self, args):

        # Get all vars from args.
        self.port_name      = args.name
        self.root_dir       = os.environ.get(args.root)
        self.data_path      = args.data
        self.save_path      = args.save + "_{}".format(args.frame)
        self.amp_prefix     = args.ampp
        self.env_prefix     = args.envp
        self.original_frame = args.origin
        self.frame_length   = args.frame
        self.sampling_rate  = args.rate
        self.movements      = args.move
        self.strategy       = args.strat
        self.overlap        = args.over
        self.threshold      = args.thresh
        self.play_back      = args.play
        self.connect_all    = args.connect


        self.save_name_amp_allo = "{}AlloMat".format(self.amp_prefix)
        self.save_name_env_allo = "{}AlloMat".format(self.env_prefix)
        #self.save_name_bayes = "{}BayesMat".format(self.prefix)
        #self.save_name_power = "{}PowerMat".format(self.prefix)        

        self.source_dir = os.path.join(self.root_dir, self.data_path)
        self.target_dir = os.path.join(self.root_dir, self.save_path)

        if not os.path.exists(self.target_dir):
            os.makedirs(self.target_dir)

        print("Source Directory : {}".format(self.source_dir))
        print("Target Directory : {}".format(self.target_dir), "\n\n")

        # Send audio to the processor.
        self.audio_out_port = yarp.Port()
        self.audio_out_port.open(self.port_name + "/rawAudio:o")

        # For reading in the Allocentric Map.
        self.allo_amp_matrix_in_port = yarp.Port()
        self.allo_amp_matrix_in_port.open(self.port_name + "/allo_amp_matrix:i")
        
        self.allo_env_matrix_in_port = yarp.Port()
        self.allo_env_matrix_in_port.open(self.port_name + "/allo_env_matrix:i")


        # For reading in the Bayesian map.
        #self.bayes_matrix_in_port = yarp.Port()
        #self.bayes_matrix_in_port.open(self.port_name + "/bayes_matrix:i")

        # For flushing the Bayesian Map.
        #self.bayes_out_port = yarp.Port()
        #self.bayes_out_port.open(self.port_name + "/bayesClear:o")

        # For reading in the Bayesian map.
        #self.power_matrix_in_port = yarp.Port()
        #self.power_matrix_in_port.open(self.port_name + "/power_matrix:i")

        # For flushing the Power Map.
        #self.power_out_port = yarp.Port()
        #self.power_out_port.open(self.port_name + "/powerClear:o")

        # For Sending Head Positions.
        if self.movements:
            self.head_out_port = yarp.Port()
            self.head_out_port.open(self.port_name + "/headAngle:o")

        # Connect all ports if enabled.
        if self.connect_all:
            if self.movements: yarp.Network.connect(self.port_name + "/headAngle:o", "/audioPreprocessor/headAngle:i")
            yarp.Network.connect(self.port_name + "/rawAudio:o",                     "/audioPreprocessor/rawAudio:i")
            yarp.Network.connect("/audioPreprocessor/allocentricAudio:o",            self.port_name + "/allo_amp_matrix:i")
            yarp.Network.connect("/audioPreprocessor/allocentricEnvelope:o",         self.port_name + "/allo_env_matrix:i")

            #yarp.Network.connect("/audioBayesianMap/bayesianProbabilityMap:o", self.port_name + "/bayes_matrix:i")
            #yarp.Network.connect(self.port_name + "/bayesClear:o",   "/audioBayesianMap")
            #yarp.Network.connect("/audioPowerMap/probabilityPowerMap:o",       self.port_name + "/power_matrix:i")
            #yarp.Network.connect(self.port_name + "/powerClear:o",   "/audioPowerMap")


    def run(self):

        #self.processing() # yarpPan_0001_1_2048_2_349.data && yarpSound_0001_1_2048_2_349.data

        while True:

            if self.play_back:
                yarp.Network.connect("/audioRunner/rawAudio:o", "/yarphear")
                self.playback()
                print("\n\nAll Data Streamed. Good Bye!\n")
                exit()

            #self.bayes_matrix_in_port.getInputCount()  and \
            #self.bayes_out_port.getOutputCount()       and \
            elif self.audio_out_port.getOutputCount()            and \
                 self.allo_amp_matrix_in_port.getInputCount()    and \
                 self.allo_env_matrix_in_port.getInputCount()    and \
                 ((not self.movements) or self.head_out_port.getOutputCount()):  

                self.processing()
                print("\n\nAll Data Processed. Good Bye!\n")
                exit()
            
            else:
                msg = "Missing Connection to "
                if not self.audio_out_port.getOutputCount():            msg += "audio output; "
                if not self.allo_amp_matrix_in_port.getInputCount():    msg += "allo amp input; "
                if not self.allo_env_matrix_in_port.getInputCount():    msg += "allo env input; "
                #if not self.bayes_matrix_in_port.getInputCount():  msg += "bayes matrix; "
                #if not self.bayes_out_port.getOutputCount():       msg += "bayes clear; "
                #if not self.power_matrix_in_port.getInputCount():  msg += "power matrix; "
                #if not self.power_out_port.getOutputCount():       msg += "power clear; "
                if self.movements and not self.head_out_port.getOutputCount():
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
            
            AlloAmpTarget_name  = self.save_name_amp_allo + target_name.replace(".data", ".npy")
            AlloEnvTarget_name  = self.save_name_env_allo + target_name.replace(".data", ".npy")
            #BayesTarget_name = self.save_name_bayes + target_name.replace(".data", ".npy")
            #PowerTarget_name = self.save_name_power + target_name.replace(".data", ".npy")

            AlloAmpTarget = os.path.join(self.target_dir, AlloAmpTarget_name)
            AlloEnvTarget = os.path.join(self.target_dir, AlloEnvTarget_name)
            #BayesTarget = os.path.join(self.target_dir, BayesTarget_name)  
            #PowerTarget = os.path.join(self.target_dir, PowerTarget_name)  
            
            print("Processing {:04d} : {}  ==>  {}, {}".format(count, sound_name, AlloAmpTarget_name, AlloEnvTarget_name), end="   ", flush=True)
            #print("Processing {:04d} : {}  ==>  {}".format(count, sound_name, BayesTarget_name), end="   ", flush=True)
            #print("Processing {:04d} : {}  ==>  {}, {}".format(count, sound_name, BayesTarget_name, PowerTarget_name), end="   ", flush=True)

            # Begin Reading in Data.
            RawData = np.loadtxt(sound_source, dtype=np.int)
            RawPos  = np.loadtxt(position_source, dtype=np.float32).tolist() if self.movements else None

            # Shape this data.
            L_RawData, R_RawData, RawPos = self._trial_process(RawData, RawPos)
            numFrames = L_RawData.shape[0]

            # Keep time.
            timeStamp = yarp.Stamp()
            timeStamp.update()

            # Clear the Bayesian & Power Map.
            #command = yarp.Bottle()
            #command.clear()
            #command.addString("clear")
            #self.bayes_out_port.write(command)
            #self.power_out_port.write(command)

            # Buffer for returned matrices.
            AlloAmpMatrixBuffer = []
            AlloEnvMatrixBuffer = []
            #BayesMatrixBuffer = []
            #PowerMatrixBuffer = []

            yarp_allo_amp_matrix = yarp.Matrix()
            yarp_allo_env_matrix = yarp.Matrix()

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
                    headPos.addDouble(float(RawPos[idx]))
                    self.head_out_port.write(headPos)

                # Wait for a response.
                self.allo_amp_matrix_in_port.read(yarp_allo_amp_matrix)
                self.allo_env_matrix_in_port.read(yarp_allo_env_matrix)

                # Append this matrix to the buffer.
                AlloAmpMatrixBuffer.append(self._matrix_process(yarp_allo_amp_matrix))
                AlloEnvMatrixBuffer.append(self._matrix_process(yarp_allo_env_matrix))

                # Update the time stamp.
                timeStamp.update()

            # Save the matrix out.
            npAlloAmp = np.asarray( AlloAmpMatrixBuffer )
            npAlloEnv = np.asarray( AlloEnvMatrixBuffer )

            np.save(AlloAmpTarget, npAlloAmp)
            np.save(AlloEnvTarget, npAlloEnv)

            count  += 1
            endTime = time.time()
            #print("\u0394 : {:.4f} || {}, {}".format(endTime-startTime, npBayes.shape, npPower.shape))
            #print("\u0394 : {:.4f} || {}".format(endTime-startTime, npBayes.shape))
            print("\u0394 : {:.4f} || {}".format(endTime-startTime, npAlloEnv.shape))



    def playback(self):

        root  = list(os.walk(self.source_dir))[0][0]
        files = sorted(list(os.walk(self.source_dir))[0][2])

        files_sound = []
        [ files[idx].startswith('yarpSound') and files_sound.append(files[idx]) for idx in range(len(files)) ]
        files_sound = sorted(files_sound)

        files_pos = []
        [ files[idx].startswith('yarpPan') and files_pos.append(files[idx]) for idx in range(len(files)) ]
        files_pos = sorted(files_pos)


        print(files_sound)

        while True:
            
            idx = int(input("\nPick a Trial in range [{}, {}] : ".format(1, len(files))))
            
            if idx < 1 or idx > len(files):
                print("Index {} out of range.".format(idx))
                continue

            sound_source = files_sound[idx-1]
            position_source = files_pos[idx-1]

            print("Streaming {:20s} ".format(sound_source), end="   ", flush=True)

            sound_source = os.path.join(root, sound_source)
            position_source = os.path.join(root, position_source)

            # Begin Reading in Data.
            RawData = np.loadtxt(sound_source, dtype=np.int)
            RawPos  = np.loadtxt(position_source, dtype=np.float32).tolist() if self.movements else None

            # Shape this data.
            L_RawData, R_RawData, RawPos = self._trial_process(RawData, RawPos)
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
                #time.sleep(5)

                # Update the time stamp.
                timeStamp.update()
            
            endTime = time.time()
            print("\u0394 : {:.4f}".format(endTime-startTime))


    def _trial_process(self, RawData, RawPos):       

        Pos_Data = None

        # Strategy 1.
        #  - Use All data, and pad the last elements for desired frame length.
        #  - Take Mean of head positions per frame.
        if self.strategy == 1:

            ogSamples = RawData.shape[1]
            numFrames = int( np.ceil(ogSamples / self.frame_length) )

            Left_Data  = np.zeros( (numFrames * self.frame_length), dtype=np.int )
            Right_Data = np.zeros( (numFrames * self.frame_length), dtype=np.int )

            Left_Data[:ogSamples ] = RawData[0]
            Right_Data[:ogSamples] = RawData[1]

            Left_Data  = Left_Data.reshape(  (numFrames, self.frame_length) )
            Right_Data = Right_Data.reshape( (numFrames, self.frame_length) )

            if self.movements:
                
                # Transform the Raw Positions so that each 'sample' 
                # in the trial has its own head position.
                # Reshape so they are concurrent.
                RawPos = RawPos * self.original_frame
                RawPos = np.asarray(RawPos, dtype=np.float32).reshape(-1,self.original_frame,order='F').reshape(-1,order='C')

                # Reshape so the head position samples are similar in shape to the audio.
                Pos_Data = np.zeros( (numFrames * self.frame_length), dtype=np.float32 )
                Pos_Data[:ogSamples] = RawPos
                
                # Fill the padding with the last sample instead of zeros.
                remaining = (numFrames * self.frame_length) - ogSamples
                if remaining != 0:
                    Pos_Data[-remaining:] = RawPos[-1]

                # Make the same shape as the audio, and take the mean of each frames samples.
                Pos_Data = Pos_Data.reshape( (numFrames, self.frame_length) )
                Pos_Data = np.mean( Pos_Data, axis=1 )


        # Strategy 2.
        #  - Window over the data.
        #  - Only use frames where the head difference was below a threshold. 
        elif self.strategy == 2:

            ogSamples = RawData.shape[1]
            
            Left_List  = RawData[0].tolist()
            Right_List = RawData[1].tolist()

            if self.movements:
                RawPos = RawPos * self.original_frame
                RawPos = np.asarray(RawPos, dtype=np.float32).reshape(-1,self.original_frame,order='F').reshape(-1,order='C')
                Pos_List = RawPos.tolist()

            Left_Buffer  = []
            Right_Buffer = []
            Pos_Buffer   = []
            
            for idx in range(0, ogSamples, self.frame_length-self.overlap):
                
                Left_Slice  = Left_List[idx:idx+self.frame_length]
                Right_Slice = Right_List[idx:idx+self.frame_length]
                
                if self.movements:
                    Pos_Slice = Pos_List[idx:idx+self.frame_length]

                    if np.abs( np.max(Pos_Slice) - np.min(Pos_Slice) ) < self.threshold and \
                         len(Left_Slice) == self.frame_length:

                        Left_Buffer.append(Left_Slice)
                        Right_Buffer.append(Right_Slice)
                        Pos_Buffer.append(np.mean(Pos_Slice))

                elif len(Left_Slice) == self.frame_length:
                    Left_Buffer.append(Left_Slice)
                    Right_Buffer.append(Right_Slice)

            Left_Data  = np.asarray(Left_Buffer,  dtype=np.int)
            Right_Data = np.asarray(Right_Buffer, dtype=np.int)
            if self.movements:
                Pos_Data = np.asarray(Pos_Buffer, dtype=np.float32)

            #print(Pos_Data.shape)
            #plt.figure()
            #plt.plot(Pos_Data, 'o')
            #plt.show()


        return Left_Data, Right_Data, Pos_Data


    def _sound_process(self, Left_Ch, Right_Ch):

        # Set some params.
        numChan = 2
        numSamp = Left_Ch.shape[0]

        yarp_sound = yarp.Sound()
        yarp_sound.resize(numSamp, numChan)
        yarp_sound.setFrequency(self.sampling_rate)

        for idx in range(numSamp):
            yarp_sound.set(int(Left_Ch [idx]), int(idx), 0)
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
        self.allo_amp_matrix_in_port.close()
        self.allo_env_matrix_in_port.close()
        #self.bayes_matrix_in_port.close()
        #self.bayes_out_port.close()
        #self.power_matrix_in_port.close()
        #self.power_out_port.close()
        if self.movements:
            self.head_out_port.close()


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

