import numpy as np
import matplotlib.pyplot as plt

import os
import time
import yarp

import argparse

# Init Yarp.
yarp.Network.init()

def get_args():
    parser = argparse.ArgumentParser(description='probabilityVisualization')
    parser.add_argument('-n', '--name',    default='/probVis',                 help='Name for the module.         (default: {})'.format('/probVis'))
    parser.add_argument('-v', '--verbose', default=False, action='store_true', help='Print out the yaw position.  (default: {})'.format(False))
    args = parser.parse_args()
    
    return args


class probabilityVisualization(object):

    def __init__(self, args):

        # Get all vars from args.
        self.port_name = args.name
        self.verbose   = args.verbose
        
        # Open all ports.
        self.input_port = yarp.Port()
        self.input_port.open(self.port_name + ":i")

        # Create a plot.
        self.fig = plt.figure()
        self.fig.canvas.set_window_title(self.port_name)
        plt.subplots_adjust( top=1.0, bottom=0.0, left=0.0, right=1.0, hspace=0.2, wspace=0.2 )
        

    def run(self):

        while True:
            if self.input_port.getInputCount():  
                self.processing()
                
            else:
                msg = "Missing Connection to "
                if not self.input_port.getInputCount():    msg += "prob input; "
                
                msg += ". . ."
                print(msg)
                time.sleep(1)
        

    def processing(self):

        # Read in a head position.
        input_matrix = yarp.Matrix()
        self.input_port.read(input_matrix)
        
        # Process the received matrix into a numpy array.
        numpy_mat = self._matrix_process(input_matrix)

        # Print out to user what the shape received was.
        if self.verbose: print("Matrix received has dim {}".format(numpy_mat.shape))

        # Plot the data.
        self.fig.clear()
        plt.xlim(0,360)
        plt.grid(color='grey', linestyle='-', linewidth=0.75, alpha=0.5)
        for mat in numpy_mat:
            plt.plot(mat)
        plt.pause(0.0005)

        
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
        self.input_port.close()
        
        

def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the visualization module.
    probVis = probabilityVisualization(args)

    # Begin running the module.
    try:
        probVis.run()

    finally:
        probVis.cleanup()
        

if __name__ == '__main__':
    main()

