import numpy as np
from scipy import interpolate
from scipy.signal import hilbert
import matplotlib.pyplot as plt
import pyaudio
import yarp
import csv

import argparse

# Init Yarp.
yarp.Network.init()

def get_args():
    parser = argparse.ArgumentParser(description='player')
    parser.add_argument('--name',  default="/audioPlayer",    help='Name for the module.                 (default: {})'.format("audioPlayer"))
    parser.add_argument('--rate',  default=48000, type=int,  help='Sampling rate.                       (default: {})'.format(48000))
    parser.add_argument('--nchan', default=2,     type=int,  help='Number of Audio Channels in Array.   (default: {})'.format(2))
    
    args = parser.parse_args()
    return args


class audioPlayer(object):

    def __init__(self, args):

        # Parse the Arguments.
        self.port_name = args.name

        self.numChannels  = args.nchan
        self.samplingRate = args.rate
        

        # For paFloat32 sample values must be in range [-1.0, 1.0]
        self.audioDevice = pyaudio.PyAudio()
        self.audioStream = self.audioDevice.open (
            format=pyaudio.paFloat32,
            channels=self.numChannels,
            rate=self.samplingRate,
            output=True
        )
        
        


        self.input_port = yarp.RpcServer()
        self.input_port.open(self.port_name + "/rpc:i")

        #reader = csv.reader(open(csv_filename, "rb"), delimiter=",")
        #data   = list(reader)
        #print(data)


    def run(self):

        # Initialize yarp containers
        command = yarp.Bottle()
        reply   = yarp.Bottle()

        # Loop until told to quit.
        while True:

            self.input_port.read(command, True)
            reply.clear()

            cmd = command.get(0).asString()
            len = command.size()

            if cmd == "quit":
                reply.addString("Good Bye!")
                self.input_port.reply(reply)
                return

            elif cmd == "help":
                reply.addString("Commands: trial <int> | play <-1:all or {}>  <string> | quit".format(list(ch for ch in range(self.numChannels))))
            
            elif cmd == "trial":
                reply.addString("please trial me!")
                self.trial(0)
            
            elif cmd == "play":
                reply.addString("please play me!")
                self.play(-1, "None")
            
            elif len == 0: continue
            else:
                reply.addString("Unkown")
                reply.addString("Command")
                reply.append(command)

            # Reply back on the rpc port.
            self.input_port.reply(reply)



    def trial(self, num):
        print("Begin Trial {}!".format(num))
        return

    
    def play(self, ch, wav_file):
        print("Begin Playing {} on channel {}".format(wav_file, ch))
        return



    def cleanup(self):
        print("Closing RPC Server.")
        self.input_port.close()

        print("Releasing PortAudio Device.")
        self.audioStream.stop_stream()
        self.audioStream.close()
        self.audioDevice.terminate()
        

def main():
    
    # Get parameters from a user.
    args = get_args()
    
    # Initialize the audio player module.
    player = audioPlayer(args)

    # Loop.
    try:
        player.run()
    finally:
        player.cleanup()
        

if __name__ == '__main__':
    main()