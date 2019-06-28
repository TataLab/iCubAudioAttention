function [ paoutput] = OpenAudioOutput(P)
%OPENAUDIOOUTPUT 
%Open an output port using Psychophysics Toolbox
%return the port

display(P.sendAngleToYarp);

InitializePsychSound(1);
%paoutput = PsychPortAudio('Open', P.outputDeviceID, P.outputMode, P.outputReqLatencyClass, P.outputSampleRate,P.outputNumChans ,P.outputBufferSize);

paoutput=PsychPortAudio('Open', [], 1, 2, 48000, 2,1024);

%PsychPortAudio('UseSchedule',paoutput,1);  %set to enable scheduling of audio data..we use this to queue frames
%make a vector to preload into the buffer
pre=zeros(P.outputNumChans,P.outputBufferSize);
PsychPortAudio('FillBuffer', paoutput, pre);


end

