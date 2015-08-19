

%Captures audio by opening a pre-recorded file and scaning through it
%frame-by-frame.  Exposes the frame to other matlab processes by memory
%mapping it as a file.  Note that the path to your audio data files will be
%different



[s,sampleRate]=readwav('/Users/Matthew/Documents/MATLAB/sounds/Sine_440hz_44100fs_7sec.wav');
