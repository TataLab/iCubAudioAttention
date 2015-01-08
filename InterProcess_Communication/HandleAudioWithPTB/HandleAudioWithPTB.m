 %HandleAudio
%This is the run loop
%It grabs audio data in chuncks (size and duration specificied in P) and
%then fills a data array and a memory mapped data array

%note there is some important timing to consider here.  This loop works on
%chuncks of data that come in at a certain rate.  Keep that in mind if you
%do any work on these data as they come in.  Better to use a multithreading
%sort of approach

%set up parameters
HandleAudioWithPTBSetParameters;

%get the audio files ready
global audioD;
global sampleD;

%create and initialize the audioDataDump file
%and the MostRecentSample file
display('creating empty audioDataDumpFile');

fid=fopen(P.audioDataDumpFilename,'w');

if(P.outputBitDepth_bytes==2)
    tempZeros=int16(zeros(P.numChannels,P.sessionDuration_samples));
else
    display('Sorry we cannot handle output other than 16-bit right now');
end

fwrite(fid,tempZeros,'int16'); %fill the audioDataDump file with zeros to pre-initialize it
fclose(fid);

fid=fopen(P.mostRecentSampleFilename,'w');
tempZero=int32(zeros(1,1));
fwrite(fid,tempZero,'int32');
fclose(fid);

clear fid
clear tempZeros
clear tempZero


[audioD,sampleD]=OpenAudioOutputData;  %note that OpenAudioInputData just memmaps some files and returns them by reference, We're doing the oppositish thing

initialAudioData = single(zeros(2,P.sessionDuration_samples)); %initialization of the audio file is to fill it with zeros
initialIndexData=int32(1);  %initialize the sample index at 1

% Perform basic initialization of the sound driver and get a handle to an
% audio device
InitializePsychSound;
pahandle = PsychPortAudio('Open', [], 2, 1, P.kSampleRate, 2,P.kNumSamples);

pause(0.5); %give MATLAB a moment to itself to settle down

% Preallocate an internal audio recording  buffer with a capacity of 1
% second (must be >> than kNumSampels) and get capture ready
%note min and max frame size is set to buffer size to keep everything tight
%(so be careful!)
PsychPortAudio('GetAudioData',pahandle, 1,P.kFrameDuration_seconds, P.kFrameDuration_seconds,1);

%start the audio stream
PsychPortAudio('Start', pahandle, 0, 0, 1); %note that setting the waitForStart flag to 1 here causes this to block until audio capture is started

currentOffset=1;
done=0;

display('reading audio data...');



while (~done) %loop to get data...note it will overflow the memory mapped file if you let it run too long
    
    t=tic;
    [audiodata,currentOffset,overflow,~] = PsychPortAudio('GetAudioData', pahandle,[],[]); %populate the indices, note the types don't match....hmmmmm
    
    %scale the audio because PTB returns values between +/- 1.0
    audiodata=audiodata.*2^(P.outputBitDepth_bytes*8-1); 
    
    currentIndex=currentOffset+1; %it's not very useful in MATLAB if it starts at zero!?
    returnedFrameSize=size(audiodata,2); %should be P.kNumSamples, but if not, the chunk should still get put in the right spot
    
    %sometimes PortAudio returns a short frame (wtf!?)
    wtfDiff=P.kNumSamples-returnedFrameSize;
    if(wtfDiff~=0)
        display(['You have a problem: the audio data is the wrong size by ' num2str(wtfDiff) ' samples']);
    end
    
%     %set up a variable for simulink to suck up
%     timeVector=linspace(0,returnedFrameSize/P.kSampleRate,returnedFrameSize);
%     simLinkData=[timeVector' audiodata']; %package for simulink
%     sim('SimLink_VisualizingASound');
    
    audioD.data(1,1).d(:,currentIndex:currentIndex+returnedFrameSize-1) = audiodata; %importantly, also stick it into the memory mapped file
    sampleD.data(1,1).f(1,1)=currentIndex+returnedFrameSize-1; %note which sample was the most recent
%     
%     figure(1);
%     plot(audiodata(1,:));
%     drawnow;
    
    %display(['length of audio frame is ' num2str(length(audiodata))]);

    
    while(toc(t)<(1/P.frameRate))
        %block for the rest of the frame duration
        
        %this is more accurate than letting PsychPortAudio's 'GetAudioData'
        %handle the timing because that fails to account for other
        %operations.  This ensures frames are (almost) exactly the right
        %duration
    end  

    %display(['the most recent sample was ' num2str(currentIndex+returnedFrameSize-1) ]);

end


% Close the audio device:
PsychPortAudio('Close', pahandle);

ok=1;


