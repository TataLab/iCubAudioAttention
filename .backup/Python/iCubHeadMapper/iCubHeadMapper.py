import numpy as np
import matplotlib.pyplot as plt

import os
import time
import yarp

import argparse

# Init Yarp.
yarp.Network.init()

def get_args():
    parser = argparse.ArgumentParser(description='iCubHeadMapper')
    parser.add_argument('-n', '--name',    default='/iCubHeadMapper', help='Name for the module.                                       (default: {})'.format('/iCubHeadMapper'))
    parser.add_argument('-y', '--yaw',     default=0, type=int,       help='Index the yaw value can be found at.                       (default: {})'.format(0))
    parser.add_argument('-v', '--verbose', default=False, action='store_true', help='Print out the yaw position.                       (default: {})'.format(False))
    args = parser.parse_args()
    
    return args


class iCubHeadMapper(object):

    def __init__(self, args):

        # Get all vars from args.
        self.port_name = args.name
        self.yaw_idx   = args.yaw
        self.verbose   = args.verbose

        # Open all ports.
        self.input_port = yarp.Port()
        self.input_port.open(self.port_name + "/state:i")
    
        self.output_port = yarp.Port()
        self.output_port.open(self.port_name + "/state:o")


    def run(self):

        while True:
            if self.input_port.getInputCount() and self.output_port.getOutputCount():  
                self.processing()
                
            else:
                msg = "Missing Connection to "
                if not self.input_port.getInputCount():    msg += "head input; "
                if not self.output_port.getOutputCount():  msg += "head output; "
                
                msg += ". . ."
                print(msg)
                time.sleep(1)
        

    def processing(self):

        # Read in a head position.
        input_bottle = yarp.Bottle()
        self.input_port.read(input_bottle)

        # Initialize an output bottle.
        output_bottle = yarp.Bottle()
        output_bottle.clear()

        # Pull the yaw position from the bottle.
        yaw = input_bottle.get(self.yaw_idx).asDouble()

        # Write the output bottle with `home` state values for all other joints.
        output_bottle.addDouble( float(0.0) ) # Pitch
        output_bottle.addDouble( float(0.0) ) # Roll
        output_bottle.addDouble( float(yaw) ) # Yaw
        output_bottle.addDouble( float(0.0) ) # Eye-Gaze 1
        output_bottle.addDouble( float(0.0) ) # Eye-Gaze 2
        output_bottle.addDouble( float(0.0) ) # Eye-Gaze 3
        
        # Write the remapped head position onto the network.
        self.output_port.write(output_bottle)

        # Give some info to user.
        if self.verbose: print("Yaw Position at {}".format(yaw))


    def cleanup(self):
        print("Closing YARP Ports.")
        self.input_port.close()
        self.output_port.close()
        
        

def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the mapper module.
    mapper = iCubHeadMapper(args)

    # Begin running the module.
    try:
        mapper.run()

    finally:
        mapper.cleanup()
        

if __name__ == '__main__':
    main()

