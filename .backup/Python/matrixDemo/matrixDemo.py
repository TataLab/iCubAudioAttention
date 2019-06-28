import numpy as np
from scipy import interpolate
from scipy.signal import hilbert
import matplotlib.pyplot as plt
import yarp

import argparse

# Init a parser.
parser = argparse.ArgumentParser(description='filter')
parser.add_argument('--rate',   default=48000, type=int,  help='Sampling rate                  (default: {})'.format(48000))
parser.add_argument('--inport', default="/matrix:i",      help='Name of input port.            (default: {})'.format("/matrix:i"))
parser.add_argument('--pfrom',  default=None,             help='Input port for matrix demo. Must be specified.')

# Init Yarp.
yarp.Network.init()

class DataProcessor():

    def __init__(self, port_name):
        print("Constructor Begin.")

        # Prepare Port.
        print(" - Port Init", " "*8, ": ", sep="", end="", flush=True)
        self.input_port = yarp.Port()
        self.input_port.open(port_name)

        # Prepare Array Buffer.
        print(" - Image Buffer", " "*5, ": ", sep="", end="\n", flush=False)
        #self.yarp_image = yarp.ImageFloat()
        self.yarp_image = yarp.Matrix()

        self.matrix = np.zeros((1,1), dtype=np.float32)

        self.fig = plt.figure()
        

    def run(self):
        
        print(" - Begin Running . . . ")
        while True:
            print(" - - Waiting for Input.")
            
            # Grab the image of the network.
            self.input_port.read(self.yarp_image)

            print(" - - Got Input.")

            # Begin Processing.
            self._process()
  

    def _process(self):
        print(" - - - In Processing : Rows={}, Cols={}".format(self.yarp_image.rows(), self.yarp_image.cols()))
        
        # Make this into a numpy array
        self._convert()

        self.fig.clear()
        plt.imshow(self.matrix)
        plt.pause(0.005)


    def _convert(self):
        print(" - - - - In Convert.")
        
        numRows = self.yarp_image.rows()
        numCols = self.yarp_image.cols()

        #x = self.yarp_image.toString(58, 1)
        x = self.yarp_image.toString(10, 1)
        self.matrix = np.fromstring(x, sep=' ').reshape((numRows,numCols))
        print(self.matrix.shape)

        
    def cleanup(self):
        print("Begin Cleanup . . . ")
        self.input_port.close()


def main():

    # Parse the arguments.
    args = parser.parse_args()

    # Ensure from is specified.
    input_from = args.pfrom
    assert input_from is not None, "--pfrom must be specified."
    
    # Grab other vars. 
    samplingRate = args.rate
    input_port   = args.inport

    # Give info to user.
    print("\n")
    print(" "*9, "[Matrix Demo]"); print(" "*3, "="*25)
    print(" -- Sampling Rate     : {}".format(samplingRate))
    print(" -- Receiving From    : {}".format(input_from))
    print(" -- Input Port Name   : {}".format(input_port))
    print("\n")
    
    matrixDemo = DataProcessor(input_port)

    try:
        assert yarp.Network.connect(input_from, input_port), "Cannot make connection from %s to %s" % (input_from, input_port)
        matrixDemo.run()

    finally:
        matrixDemo.cleanup()


if __name__ == '__main__':
    main()
