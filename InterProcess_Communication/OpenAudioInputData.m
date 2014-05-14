function [audioD,sampleD]=OpenAudioInputData
%memory map the file that is being written to by the iCub audio data stream
%map it onto a channels x samples array
global P;

display(['memory mapping file ' P.audioDataDumpFilename ' for audio data.']);
display(['memory mapping file ' P.mostRecentSampleFilename ' for sample index.']);

%get the size of the audio data file
f=dir(P.audioDataDumpFilename);
P.sessionDuration_bytes = f.bytes;
P.sessionDuration_samples=P.sessionDuration_bytes/P.numChannels/P.bitDepth_bytes;

audioD  = memmapfile(P.audioDataDumpFilename, 'Writable', true, 'format',{'int16' [P.numChannels P.sessionDuration_samples] 'd'});
sampleD = memmapfile(P.mostRecentSampleFilename, 'Writable', true, 'format',{'int32' [1 1] 'f'});
end


