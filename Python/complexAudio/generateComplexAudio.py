import numpy as np
import soundfile as sf

import os
import argparse

def get_args():
    parser = argparse.ArgumentParser(description='complex')
    parser.add_argument('-r', '--rate',  default=48000,  type=int,            help='Sampling Rate of Audio.               (default: {})'.format(48000))
    parser.add_argument('-t', '--time',  default=15.0,   type=float,          help='Time in seconds to generate.          (default: {})'.format(15.0))
    parser.add_argument('-m', '--mod',   default=5.0,    type=float,          help='Frequency to modulate at.             (default: {})'.format(5.0))
    parser.add_argument('-b', '--band',  default=64,     type=int,            help='Number of Center Frequencies.         (default: {})'.format(64))
    parser.add_argument('-L', '--low',   default=300.0,  type=float,          help='Low Center Frequency.                 (default: {})'.format(300.0))
    parser.add_argument('-H', '--high',  default=4800.0, type=float,          help='High Center Frequency.                (default: {})'.format(4800.0))
    parser.add_argument('-s', '--space', default=2,      type=int,            help='Frequency Band Spacing Used.          (default: {})'.format(2))
    parser.add_argument('-l', '--lin',   default=False,  action='store_true', help='Make Linear Spaced.                   (default: {})'.format(False))
    parser.add_argument('-S', '--save',  default='output.wav',                help='Name of the output file.              (default: {})'.format('output.wav'))
    args = parser.parse_args()
    return args

def genPureTone(freq, rate, sec):
    t = 2*np.pi * np.arange(rate * sec) / rate
    pureTone = np.sin(freq * t).astype(np.float32)
    return pureTone

def genComplex(cFreq, rate, sec):
    complexTone = np.zeros( int(rate*sec), dtype=np.float32 )
    for freq in cFreq:
        complexTone += genPureTone(freq, rate, sec)
    return complexTone

def ampModulate(source, freq, rate):
    length = source.shape[0]
    t = 2*np.pi * np.arange(length) / rate
    e = np.sin(freq * t) + 1
    return e * source

def HzToErb(Hz):
    return ( 24.7 * ( 0.00437 * Hz + 1.0 ) )

def HzToErbRate( Hz):
    return ( 21.4 * np.log10( 0.00437 * Hz + 1.0 ) )

def ErbRateToHz(Erb):
    return ((10**(Erb / 21.4)) - 1.0) / 0.00437
  
def makeErbCFs(numBands, spacing, lowCf, highCf):

    #-- Make sure space is allocated for the 
    #-- center frequency table.
    cfs = []

    #-- Calculates the lower bound in ERB space.
    lowERB = HzToErbRate(lowCf)

    #-- Calculates the upper bound in ERB space.
    highERB = HzToErbRate(highCf)

    #-- Calculates the incrementing amount.
    linspace_step = (highERB - lowERB) / (numBands - 1.0)
    current_step  = lowERB

    for band in range(numBands):
        if (band % spacing) == 0:
            cfs.append( ErbRateToHz(current_step) )
        current_step += linspace_step

    return cfs


def makeLinearCFs(numBands, spacing, lowCf, highCf):

    #-- Make sure space is allocated for the 
    #-- center frequency table.
    cfs = []

    #-- Calculates the incrementing amount.
    linspace_step = (highCf - lowCf) / (numBands - 1.0)
    current_step  = lowCf

    for band in range(numBands):
        if (band % spacing) == 0:
            cfs.append( current_step )
        current_step += linspace_step

    return cfs


def main():
    args = get_args()
    if args.lin: cFreq = makeLinearCFs(args.band, args.space, args.low, args.high)
    else:        cFreq = makeErbCFs(args.band, args.space, args.low, args.high)

    tone    = genComplex(cFreq, args.rate, args.time)
    ampTone = ampModulate(tone, args.mod, args.rate)

    ampTone *= ( 1 / np.max(ampTone) )

    sf.write(args.save, ampTone, args.rate)


if __name__ == '__main__':
    main()
