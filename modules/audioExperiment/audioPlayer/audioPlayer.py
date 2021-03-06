import numpy as np
import librosa
import soundfile as sf
import matplotlib.pyplot as plt

import csv
import os
import pyaudio
import time
import yarp

import argparse

# Init Yarp.
yarp.Network.init()

def get_args():
    parser = argparse.ArgumentParser(description='player')
    parser.add_argument('-n', '--name',    default="/audioPlayer",              help='Name for the module.                          (default: {})'.format("audioPlayer"))
    parser.add_argument('-r', '--rate',    default=48000,  type=int,            help='Sampling rate.                                (default: {})'.format(48000))
    parser.add_argument('-c', '--chan',    default=2,      type=int,            help='Number of Audio Channels in Array.            (default: {})'.format(2))
    parser.add_argument('-s', '--special', default=False,  action='store_true', help='Use special setting setting for our set up.   (default: {})'.format(False))
    parser.add_argument('-v', '--csv',     default='None',                      help='CSV that contains trail information.')
    args = parser.parse_args()
    return args


class audioPlayer(object):

    def __init__(self, args):

        # Init the Yarp Port.
        self.port_name = args.name

        self.input_port = yarp.RpcServer()
        self.input_port.open(self.port_name + "/rpc:i")
        
        self.output_port = yarp.BufferedPortBottle()
        self.output_port.open(self.port_name + "/broadcast:o")


        # Init PortAudio Device.
        if args.special:
            self.samplingRate = 48000
            self.numChannels  = 10
            self.speakerMap   = list( range(2,10) )
        else:
            self.samplingRate = args.rate
            self.numChannels  = args.chan
            self.speakerMap   = list( range(args.chan) )


        # For paFloat32 sample values must be in range [-1.0, 1.0]
        self.audioDevice = pyaudio.PyAudio()
        self.audioStream = self.audioDevice.open (
            format=pyaudio.paFloat32,
            channels=self.numChannels,
            rate=self.samplingRate,
            output=True
        )       

        # Read in the CSV containing trial information.
        self.data = None
        csv_filename = args.csv
        if csv_filename == 'NONE':
            print("No CSV Provided, continuing without.")
        elif not csv_filename.endswith('.csv'):
            print("{} is not a valid CSV, continuing without.".format(csv_filename))
        else:
            reader = csv.reader(open(csv_filename, "r"), delimiter=",")
            self.data = list(reader)
            

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

            # Kill this script.
            if cmd == "quit" or cmd == "--quit" or cmd == "-q" or cmd == "-Q":
                reply.addString("Good Bye!")
                #self.input_port.reply(reply) # Don't return yet.
                #return

            # Get a help Message.
            elif cmd == "help" or cmd == "--help" or cmd == "-h" or cmd == "-H":
                reply.addString( # Please don't look at this.
                    "Commands: trial <int> | play -1 <string>.wav | play {} | ping | quit ".format(
                        " ".join(list(" ".join([str(ch), '{}.wav'.format(chr(97+ch))]) for ch in range(self.numChannels)))
                    )
                )
            
            # Ping the server.
            elif cmd == "ping" or cmd == "--ping":
                reply.addString("Oll Korrect")

            # Run a trial from the provided csv.
            elif cmd == "trial" or cmd == "--trial" or cmd == "-t" or cmd == "-T":
                if len > 2:
                    reply.addString("Too many commands. Message should be ``trial <int>`` not ``{}``".format(command.toString()))
                elif len == 1:
                    reply.addString("Please indicate the trial number. Message should be ``trial <int>``")
                else:
                    # Check that trial number is valid.
                    trial_num = command.get(1).asInt()
                    if trial_num == 0:
                        reply.addString("Please ensure the trial number is an integer, not string ``{}``".format(command.get(1).asString()))
                    else:
                        targetAt = self.trial(trial_num)
                        reply.addString("Trial Complete")
                        reply.addInt(targetAt)
            
            # Play an instance of 
            elif cmd == "play" or cmd == "--play" or cmd == "-p" or cmd == "-P":
                if len % 2 == 0 or len == 1:
                    reply.addString("play command should be used as ``play -1 <string>`` or ``play <int> <string> <int> <string>``")
                else:
                    play_at  = []
                    play_wav = []
                    play_msg = ""
                    play_success = False

                    # Play at All Channels.
                    if command.get(1).asInt() == -1:
                        if len == 3:
                            play_success, play_msg = self.play(-1, command.get(2).asString())
                        else:
                            reply.addString("Too many arguments for ``play -1 <string>``")

                    # Play at Specific Channels.
                    else:
                        for idx in range(1, len, 2):
                            play_at.append(command.get(idx).asInt())
                            play_wav.append(command.get(idx+1).asString())
                        play_success, play_msg = self.play(play_at, play_wav)

                    # Prep the response.
                    if play_success:
                        reply.addString("Play Complete")
                    else:
                        reply.addString("Error in Play") 
                        reply.addString("PlayError: {}".format(play_msg))


            # Catch Unknown Commands.
            elif len == 0: continue
            else:
                reply.addString("Unkown")
                reply.addString("Command")
                reply.append(command)

            # Reply back on the rpc port.
            self.input_port.reply(reply)

            # Broadcast that reply if someone is connected.
            if self.output_port.getOutputCount():
                broadcast = self.output_port.prepare()
                broadcast.clear()
                broadcast.fromString(reply.toString())
                self.output_port.write()

            # Leave loop if requested.
            if reply.get(0).asString() == "Good Bye!":
                return



    def trial(self, num):
        if self.data == None:
            print("No Trials in Data . . .")
            return

        print("Begin Trial {}!".format(num))

        currentTrial = self.data[num]

        play_ch = []
        play_fn = []

        source = currentTrial[2:]

        for idx in range(len(source)):
            if source[idx] == 'NONE':
                continue
            
            play_ch.append(idx)
            play_fn.append(source[idx])

        self.play(play_ch, play_fn)

        return int(currentTrial[1])



    def play(self, channels, filename):
        print("Begin Playing . . . ")
        for f, c in zip(filename, channels):
            print("\t{} : {}".format(c, f))

        if channels == -1:
            if not os.path.isfile(filename):
                return False, "Audio File {} does not exist.".format(filename)
            
            data, samps = self.open_audio(filename)
            audioSamples = np.zeros((self.numChannels, samps), dtype=np.float32)
            audioSamples[self.speakerMap] = data
        
        else:
            if len(channels) > len(set(channels)):
                return False, "The provided channels should be unique. Was given {}".format(channels)

            maxNumSamps  = self.samplingRate
            audioSamples = np.zeros((self.numChannels, maxNumSamps), dtype=np.float32)

            for ch, fn in zip(channels, filename):

                if ch >= self.numChannels or ch < 0:
                    return False, "Channel {} is out of range [0:{}]".format(ch, len(self.speakerMap))
                if not os.path.isfile(fn):
                    return False, "Audio File {} does not exist.".format(fn)
                
                data, samps = self.open_audio(fn)

                # Resize the audioSamples if we load a file that is longer.
                if samps > maxNumSamps: 
                    resize_samples = np.zeros((self.numChannels, samps), dtype=np.float32)
                    resize_samples[:,:maxNumSamps] = audioSamples
                    audioSamples = resize_samples
                    maxNumSamps  = samps

                audioSamples[self.speakerMap[ch],:samps] = data

        # Write to stream.
        self.audioStream.write(audioSamples.transpose().tobytes())
        return True, ""



    def open_audio(self, filename):
        data, rate = sf.read(filename)
        if rate != self.samplingRate:
            data = librosa.resample(data, rate, self.samplingRate)
        
        return data, data.shape[0]



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
