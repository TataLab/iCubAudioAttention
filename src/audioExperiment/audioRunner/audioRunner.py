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
    parser.add_argument('-n', '--name',  default="/audioRunner",             help='Name for the module.                   (default: {})'.format("/audioRunner"))
    parser.add_argument('-b', '--begin', default=1,     type=int,            help='First Trial to begin at.               (default: {})'.format(1))
    parser.add_argument('-e', '--end',   default=100,   type=int,            help='Last Trial to end at.                  (default: {})'.format(100))
    parser.add_argument('-s', '--sleep', default=10,    type=int,            help='Sleep for between trials.              (default: {})'.format(10))
    parser.add_argument('-m', '--move',  default=False, action='store_true', help='Enable Periodic Movements of the Head. (default: {})'.format(False))
    args = parser.parse_args()
    return args


class audioRunner(object):

    def __init__(self, args):

        # Init the Yarp Ports.
        self.port_name   = args.name
        self.trial_begin = args.begin
        self.trial_end   = args.end
        self.sleep_for   = args.sleep
        self.movements   = args.move

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
        self.matrix_in_port = yarp.Port()
        self.matrix_in_port.open(self.port_name + "/matrix:i")

        # For clearing the Bayesian map.
        self.clear_out_port = yarp.Port()
        self.clear_out_port.open(self.port_name + "/bayesClear:o")

        # For reading in the encoder position from the robot head.
        self.head_in_port = yarp.Port()
        self.head_in_port.open(self.port_name + "/headAngle:i")

        # For sending a new position to the robot head.
        self.head_out_port = yarp.Port()
        self.head_out_port.open(self.port_name + "/headAngle:o")


    def run(self):

        # Yarp Bottle for sending commands on the network.        
        command = yarp.Bottle()

        # Loop Through the trials range.
        for currentTrial in range(self.trial_begin, self.trial_end):
            
            # At the beginning of every trial:
            
            # 1) move the head to pos 0.
            if self.movements:
                command.clear()
                # Add Bottle Pos.
                # Send Bottle.

            # 2) clear out the bayes map.
            command.clear()
            command.addString("clear")
            self.clear_out_port.write(command)

            # 3) wait for the bayes map to fill.
            command.clear()
            time.sleep(self.sleep_for) 


            # Give some Verbose of the current trial info.
            print("Sending [{} {}]".format("trial", currentTrial))
            command.addString("trial")
            command.addInt(currentTrial)

            self.rpc_out_port.write(command)

            while True:
                # While Waiting for trial to finish:

                # 1) Collect the current position of the head.
                headPos = yarp.Bottle()
                self.head_in_port.read(headPos)
                               
                # 2) Read in and save the Bayes Map as numpy obj.
                yarp_matrix = yarp.Matrix()
                self.matrix_in_port.read(yarp_matrix)

                matrix = self._mat_process(yarp_matrix)
                np.save("filename", matrix)

                # 3) See if Trial has finished.
                reply = self.rpc_in_port.read(False)
                if reply != None:
                    print(type(reply), reply.toString(), reply)
                    break
                    

    def _mat_process(self, yarp_matrix):
        
        # Get the size of the original matrix
        numRows = yarp_matrix.rows()
        numCols = yarp_matrix.cols()

        # Convert to a string, and convert to numpy matrix.
        string_mat = yarp_matrix.toString(10, 1)    # (precision, tabs:0 space:1)
        np_matrix = np.fromstring(string_mat, sep=' ').reshape((numRows,numCols))
        return np_matrix


    def cleanup(self):
        print("Closing YARP Ports.")
        self.rpc_in_port.close()
        self.rpc_out_port.close()
        self.matrix_in_port.close()
        self.clear_out_port.close()
        self.head_in_port.close()
        self.head_out_port.close()


def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the audio runner module.
    runner = audioRunner(args)

    # Make some connections relative to the module and begin running.
    try:
        name = args.name
        assert yarp.Network.connect(  name+"/rpc:o",                              "/audioPlayer/rpc:i"   ), "Cannot make connection 1"
        assert yarp.Network.connect( "/audioPlayer/broadcast:o",                   name+"/broadcast:i"   ), "Cannot make connection 2"
        assert yarp.Network.connect( "/audioBayesianMap",                          name+"/bayesClear:o"  ), "Cannot make connection 3"
        assert yarp.Network.connect( "/audioBayesianMap/bayesianProbabilityMap:o", name+"/matrix:i"      ), "Cannot make connection 4"

        runner.run()
    finally:
        runner.cleanup()
        

if __name__ == '__main__':
    main()