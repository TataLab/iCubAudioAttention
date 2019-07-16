ITDLocalizer.m and its relatives estimates the angle to a sound source.  It uses a "quasi-real-time" approach:  audio is written to a file by a YARP
helper program.  This file is memory mapped by MATLAB.  The "current" frame should be explicitly lagged behind the most recent
frame actually written into the audio data dump file.  (by how much...depends on lots...test your system for dropouts and other
flakiness).

