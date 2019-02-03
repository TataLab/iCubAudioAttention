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
    parser.add_argument('-n', '--name',    default='/audioRunner',             help='Name for the module.                       (default: {})'.format("/audioRunner"))
    parser.add_argument('-r', '--read',    default='None',                     help='Name of the port to conenct and read from. (default: {})'.format('None'))
    parser.add_argument('-b', '--begin',   default=1,     type=int,            help='First Trial to begin at.                   (default: {})'.format(1))
    parser.add_argument('-e', '--end',     default=100,   type=int,            help='Last Trial to end at.                      (default: {})'.format(100))
    parser.add_argument('-s', '--sleep',   default=10,    type=int,            help='Sleep for between trials.                  (default: {})'.format(10))
    parser.add_argument('-m', '--move',    default=False, action='store_true', help='Enable Periodic Movements of the Head.     (default: {})'.format(False))
    parser.add_argument('-R', '--robot',   default='icub',                     help='Name of the robot head to connect to.      (default: {})'.format('icub'))
    parser.add_argument('-t', '--type',    default='Matrix',                   help='Object Expected to Save. (Matrix | Sound)  (default: {})'.format('Matrix'))
    args = parser.parse_args()
    return args


class audioRunner(object):

    def __init__(self, args):

        # Init the Yarp Ports.
        self.port_name   = args.name
        self.connect_to  = args.read
        self.trial_begin = args.begin
        self.trial_end   = args.end
        self.sleep_for   = args.sleep
        self.movements   = args.move
        self.yarp_type   = args.type

        # Send Messages to audioPlayer.
        self.rpc_out_port = yarp.Port()
        self.rpc_out_port.open(self.port_name + "/rpc:o")

        # Listen for responses from audioPlayer. 
        #  This is decoupled from the rpcClient so that
        #  we can do other things while waiting for a
        #  response from audioPlayer.
        self.rpc_in_port = yarp.BufferedPortBottle()
        self.rpc_in_port.open(self.port_name + "/broadcast:i")

        # For reading in the Bayesian map.
        self.yarp_in_port = yarp.Port()
        self.yarp_in_port.open(self.port_name + "/yarp:i")

        # For reading in the encoder position from the robot head.
        self.head_in_port = yarp.Port()
        self.head_in_port.open(self.port_name + "/headAngle:i")

        # For sending a new position to the robot head.
        self.head_out_port = yarp.Port()
        self.head_out_port.open(self.port_name + "/headAngle:o")


        # Set the container for reading input.
        if   self.yarp_type == 'Matrix':
            self.yarp_input = yarp.Matrix()
        elif self.yarp_type == 'Sound':
            self.yarp_input = yarp.Sound()
        else:        
            # Handle errors.
            print("Please specify a data type to read in with -t or --type.")
            print("Current Support Includes:")
            print("\t Matrix \n\t Sound \n")
            if self.yarp_type != 'None':
                print("Unknown Type Provided: {}".format(self.yarp_type))

            self.cleanup()
            exit()

        if self.connect_to == 'None':
            print("Please specify a yarp port to read from -r or --read.")
            self.cleanup()
            exit()

        print("Constructor Finished.")


    def run(self):

        # Yarp Bottle for sending commands on the network.        
        command = yarp.Bottle()
        stamp   = yarp.Stamp()

        input_buffer = []
        time_buffer  = []


        # Loop Through the trials range.
        for currentTrial in range(self.trial_begin, self.trial_end):
            
            # At the beginning of every trial:
            
            # move the head to pos 0.
            if self.movements:
                command.clear()
                # Add Bottle Pos.
                # Send Bottle.

            # wait some small amount of time between trials.
            #time.sleep(self.sleep_for)

            # Give some Verbose of the current trial info.
            print("Sending [{} {}]".format("trial", currentTrial))

            # Send a command to the player.
            command.clear()
            command.addString("trial")
            command.addInt(currentTrial)
            self.rpc_out_port.write(command)

            # Buffer the data recieved to save
            # it as a numpy array at the end.

            while True:
                # While Waiting for trial to finish:

                # 1) Collect the current position of the head.
                #headPos = yarp.Bottle()
                #self.head_in_port.read(headPos)
                
                # 2) Read in and save the yarp object as numpy obj.
                self.yarp_in_port.read(self.yarp_input)
                self.yarp_in_port.getEnvelope(stamp)

                time_buffer.append(stamp.getCount())
                input_buffer.append(self.yarp_input)
                

                # 3) See if Trial has finished.
                reply = self.rpc_in_port.read(shouldWait=False)
                if reply != None:
                    print(reply.toString())
                    break
            


            #if self.yarp_type == 'Matrix':
            #    input_buffer.append(self._matrix_process(self.yarp_input))
            #elif self.yarp_type == 'Sound':
            #    input_buffer.append(self._sound_process(self.yarp_input))

            if currentTrial == 1:
                print(time_buffer)
                exit()

    def _matrix_process(self, yarp_matrix):
        
        # Get the size of the original matrix.
        numRows = yarp_matrix.rows()
        numCols = yarp_matrix.cols()

        # Convert to a string, and convert to numpy matrix.
        string_mat = yarp_matrix.toString(10, 1)    # (precision, tabs:0 space:1)
        np_matrix = np.fromstring(string_mat, sep=' ').reshape((numRows,numCols))
        return np_matrix


    def _sound_process(self, yarp_sound):

        # Get the info from sound.
        numChan  = yarp_sound.getChannels()
        numSamp  = yarp_sound.getSamples()

        # Convert the sound object to numpy array for storage.
        np_sound = np.zeros( (numChan, numSamp), dtype=np.int32 )

        for idx in range(numSamp):
            np_sound[0, idx] = yarp_sound.get(idx, 0)
            np_sound[1, idx] = yarp_sound.get(idx, 1)            

        return np_sound

        

        


    def cleanup(self):
        print("Closing YARP Ports.")
        self.rpc_in_port.close()
        self.rpc_out_port.close()
        self.yarp_in_port.close()
        self.head_in_port.close()
        self.head_out_port.close()


def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the audio runner module.
    runner = audioRunner(args)

    # Make some connections relative to the module and begin running.
    try:
        name  = args.name
        read  = args.read
        robot = args.robot

        assert yarp.Network.connect(  name+"/rpc:o",                "/audioPlayer/rpc:i" ), "Cannot make connection 1"
        assert yarp.Network.connect( "/audioPlayer/broadcast:o",     name+"/broadcast:i" ), "Cannot make connection 2"
        assert yarp.Network.connect(  read,                          name+"/yarp:i"      ), "Cannot make connection 3"
        if args.move:
            assert yarp.Network.connect( "/"+robot+"/head/state:o",  name+"/headAngle:i" ), "Cannot make connection 4"
            assert yarp.Network.connect(  name+"/headAngle:o",      "/audioStreamer:i"   ), "Cannot make connection 5"
        
        runner.run()

    finally:
        runner.cleanup()
        

if __name__ == '__main__':
    main()