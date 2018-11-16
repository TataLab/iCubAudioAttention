import numpy as np
from scipy import interpolate
from scipy.signal import hilbert
import matplotlib.pyplot as plt
import yarp

import argparse

# Init a parser.
parser = argparse.ArgumentParser(description='filter')
parser.add_argument('--rate',   default=48000, type=int,  help='Sampling rate                  (default: {})'.format(48000))
parser.add_argument('--inport', default="/filter:i",      help='Name of input port.            (default: {})'.format("/EnvelopeVis:i"))
parser.add_argument('--pfrom',  default=None,             help='Input port for filtering demo. Must be specified.')

# Init Yarp.
yarp.Network.init()


def genPureTone(freq, samp, rate):
    t = np.linspace(0, samp*2.0*np.pi/rate, samp)
    pureTone = np.sin(freq * t)
    return pureTone


def beamformer(filteredAudio, numBeams=43, rms=False):
    print(" - - - - In Beamformer.")

    numBands = filteredAudio.shape[0] // 2
    numSamps = filteredAudio.shape[1]

    beamsPerHemi = (numBeams-1) // 2
    
    beamformedAudio = np.zeros((numBands, numBeams, numSamps), dtype=np.float32)

    for band in range(numBands):

        ch0 = int(band)
        ch1 = int(band + numBands)

        for beam in range(numBeams):
            beamformedAudio[band][beam] = ( 
                filteredAudio[ch0] + 
                np.roll(filteredAudio[ch1], (beam - beamsPerHemi)) 
            )


    if not rms:
        return beamformedAudio
 
    beamformedAudioRms = np.sqrt(np.sum(beamformedAudio**2, axis=2)) / numSamps

    return beamformedAudioRms
    

def bandPass(Audio, CenterFreq, numSamples, rate):
    
    BW = 3 * np.pi * 1.019  # BW = n * pi * BW_CORRECTION
    C  = 1.0 / np.tan( np.pi * ( BW / rate) )
    D  = 2.0 * np.cos( np.pi * 2.0 * (CenterFreq / rate) )

    a0 =  1.0 / (C + 1.0)
    a1 =  0.0
    a2 = -1.0 * a0
    b1 = -1.0 * a0 * C * D
    b2 =  a0 * (C - 1.0)

    Filtered = np.zeros(Audio.shape, dtype=np.float32)

    p0i = p1i = p2i = 0
    p0o = p1o = p2o = 0

    for s in range(numSamples):

        p0i = Audio[:,:,s]
        p0o = ( (a0*p0i) + (a1*p1i) + (a2*p2i) ) - ( (b2*p2o) + (b1*p1o) )

        p2i = p1i; p1i = p0i
        p2o = p1o; p1o = p0o

        Filtered[:,:,s] = p0o

    return Filtered


class DataProcessor():

    def __init__(self, port_name):
        print("Constructor Begin.")

        # Prepare Port.
        print(" - Port Init", " "*8, ": ", sep="", end="", flush=True)
        self.input_port = yarp.Port()
        self.input_port.open(port_name)

        # Prepare Array Buffer.
        print(" - Image Buffer", " "*5, ": ", sep="", end="\n", flush=False)
        self.yarp_image = yarp.ImageFloat()

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
        print(" - - - In Processing : Hight={}, Width={}".format(self.yarp_image.height(), self.yarp_image.width()))
        
        # Make this into a numpy array
        self._convert()

        beamedAudio = beamformer(self.matrix, rms=False)
        numSamps = beamedAudio.shape[2]

        showEnv = True

        if showEnv:
            
            oneBeam = np.zeros((1, 1, numSamps), dtype=np.float32)
            oneBeam[0][0] = beamedAudio[7][21]

            print(" - - - - Begin Hilbert.")
            analytic_signal = hilbert(oneBeam)
            amplitude_envelope = np.abs(analytic_signal)
            band_passed_amp = bandPass(amplitude_envelope, 5.0, numSamps, 48000)
            
            amp_oneBeam = oneBeam[0][0].copy()
            amp_oneBeam[amp_oneBeam < 0.0] = 0.0

            # Plot.
            self.fig.clear()

            ax0 = self.fig.add_subplot(311)
            ax0.set_ylim(-1, 1)
            ax0.plot(oneBeam[0][0])
            ax0.plot(amp_oneBeam)


            ax1 = self.fig.add_subplot(312)
            ax1.set_ylim(-1, 1)
            ax1.plot(analytic_signal[0][0])
            ax1.plot(amplitude_envelope[0][0])
            
            ax2 = self.fig.add_subplot(313)
            ax2.set_ylim(-1, 1)
            ax2.plot(band_passed_amp[0][0])


        else:
            print(" - - - - Begin Hilbert.")
            analytic_signal    = hilbert(beamedAudio)
            amplitude_envelope = np.abs(analytic_signal)

            print(" - - - - Begin Band Pass.")
            band_passed_amp = bandPass(amplitude_envelope, 5.0, numSamps, 48000)

            reduced_band_pass = np.sqrt(np.sum(band_passed_amp**2, axis=2)) / numSamps

            self.fig.clear()
            plt.imshow(reduced_band_pass)


        plt.pause(0.005)


    def _convert(self):
        print(" - - - - In Convert.")
        
        h = self.yarp_image.height()
        w = self.yarp_image.width()

        self.matrix.resize(h, w)
        for r in range(h):
            for c in range(w):
                self.matrix[r][c] = self.yarp_image.getPixel(c, r)

        
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
    print(" "*9, "[Filter Demo]"); print(" "*3, "="*25)
    print(" -- Sampling Rate     : {}".format(samplingRate))
    print(" -- Receiving From    : {}".format(input_from))
    print(" -- Input Port Name   : {}".format(input_port))
    print("\n")
    
    filterDemo = DataProcessor(input_port)

    try:
        assert yarp.Network.connect(input_from, input_port), "Cannot make connection from %s to %s" % (input_from, input_port)
        filterDemo.run()

    finally:
        filterDemo.cleanup()


if __name__ == '__main__':
    main()
