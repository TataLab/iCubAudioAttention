import numpy as np
import matplotlib.pyplot as plt

import os
import yarp

import argparse

# Init Yarp.
yarp.Network.init()

def get_args():
    parser = argparse.ArgumentParser(description='runner')
    parser.add_argument('-n', '--name',  default="/audioRunner", help='Name for the module.             (default: {})'.format("/audioRunner"))
    parser.add_argument('-b', '--begin', default=1,   type=int,  help='First Trial to begin at.         (default: {})'.format(1))
    parser.add_argument('-e', '--end',   default=100, type=int,  help='Last Trial to end at.            (default: {})'.format(100))
    
    args = parser.parse_args()
    return args


class audioRunner(object):

    def __init__(self, args):

        # Init the Yarp Ports.
        self.port_name = args.name

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

        # For reading in the encoder position from the robot head.
        self.head_in_port = yarp.Port()
        self.head_in_port.open(self.port_name + "/headAngle:i")

        # For sending a new position to the robot head.
        self.head_out_port = yarp.Port()
        self.head_out_port.open(self.port_name + "/headAngle:o")


        # Get some information from the parser.
        self.trial_begin = args.begin
        self.trial_end   = args.end


    def run(self):
        
        command = yarp.Bottle()

        for currentTrial in range(self.trial_begin, self.trial_end):

            command.clear()

            if self.rpc_out_port.getOutputCount():
                print("Sending [{} {}]".format("trial", currentTrial))
                command.addString("trial")
                command.addInt(currentTrial)

                self.rpc_out_port.write(command)

                t1 = yarp.now()

                while True:

                    print("Working . . . ")

                    reply = self.rpc_in_port.read(False)

                    if reply != None:
                        t2 = yarp.now()
                        print(t2-t1, ":", type(reply), reply.toString(), reply)
                        break

                    
        return 

    def cleanup(self):
        print("Closing YARP Ports.")
        self.rpc_in_port.close()
        self.rpc_out_port.close()
        self.matrix_in_port.close()
        self.head_in_port.close()
        self.head_out_port.close()



        

def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the audio runner module.
    runner = audioRunner(args)

    # Loop.
    try:
        name = args.name
        assert yarp.Network.connect( name+"/rpc:o",             "/audioPlayer/rpc:i"   ),   "Cannot make connection 1"
        assert yarp.Network.connect( "/audioPlayer/broadcast:o", name+"/broadcast:i"   ),   "Cannot make connection 2"

        runner.run()
    finally:
        runner.cleanup()
        

if __name__ == '__main__':
    main()