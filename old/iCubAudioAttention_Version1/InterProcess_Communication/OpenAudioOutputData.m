function [audioD,sampleD]=OpenAudioOutputData
%This output side version of OpenAudioInputData
%memory maps a file for PTB to use as if it was streaming from the iCub
%using AudioStreamHandler.  (i.e. you don't need to call this if you're
%actually using YARP)

%map it onto a channels x samples array
global P;

display(['memory mapping file ' P.audioDataDumpFilename ' for audio data.']);
display(['memory mapping file ' P.mostRecentSampleFilename ' for sample index.']);

%get the size of the audio data file
f=dir(P.audioDataDumpFilename);
P.sessionDuration_bytes = f.bytes;
P.sessionDuration_samples=P.sessionDuration_bytes/P.numChannels/P.outputBitDepth_bytes;

%There might be something flaky about the size of the samples.  They don't
%seem to span all 16 bits
audioD  = memmapfile(P.audioDataDumpFilename, 'Writable', true, 'format',{'int16' [P.numChannels P.sessionDuration_samples] 'd'});
sampleD = memmapfile(P.mostRecentSampleFilename, 'Writable', true, 'format',{'int32' [1 1] 'f'});



end