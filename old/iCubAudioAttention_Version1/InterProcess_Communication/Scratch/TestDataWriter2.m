%test writing data using memory mapping and including a time stamp

global P;

%make a  struct to hold some parameters for convenience
P.kNumSamples = 4096;  %size of chunk of audio data to capture
P.kSampleRate = 16000; %the sampling frequency of the audio hardware
P.kChunkDuration_seconds = P.kNumSamples/P.kSampleRate; %how long each chunk takes in seconds
P.use='ptb';  % 'ptb for PsychToolBox's portaudio interface (very easy)  or 'yarp' for the iCub (um, not so easy)
%P.sessionDuration_seconds = 60*30;  %how long will we run for (to preallocate the audio data memory);

P.frameRate=P.kSampleRate/P.kNumSamples; %rate at which audio frames should be read
P.numFrameWrites = 7000;
P.sessionDuration_samples = P.numFrameWrites * P.kNumSamples;

%some MATLAB voodoo here:  instead of writing directly to the array, we'll
%map the file to memory using memmapfile().  Then as we fill it with data
%we can view and use the data from another instance of MATLAB ... a cludgy
%way to acheive multithreaded audio


audioD = int32(zeros(3,P.sessionDuration_samples)); %initialization of the audio file is to fill it with zeros

fileID=fopen('/Users/Matthew/Documents/MATLAB/Icub Interface/Memory/SensoryMemory.dat','w');
fwrite(fileID,audioD,'int32');
fclose(fileID);

audioD_memFile=memmapfile('/Users/Matthew/Documents/MATLAB/Icub Interface/Memory/SensoryMemory.dat','format',{'int32' [3 P.sessionDuration_samples] 'd'}, 'writable',true);


% Perform basic initialization of the sound driver and get a handle to an
% audio device
InitializePsychSound;
pahandle = PsychPortAudio('Open', [], 2, 1, P.kSampleRate, 2,P.kNumSamples);

pause(0.5); %give MATLAB a moment to itself to settle down

% Preallocate an internal audio recording  buffer with a capacity of 1
% second (must be >> than kNumSampels) and get capture ready
%note min and max chunk size is set to buffer size to keep everything tight
%(so be careful!)
PsychPortAudio('GetAudioData',pahandle, 1,P.kChunkDuration_seconds, P.kChunkDuration_seconds,1);

%start the audio stream
PsychPortAudio('Start', pahandle, 0, 0, 1); %note that setting the waitForStart flag to 1 here causes this to block until audio capture is started

i=1;
done=0;

display('writing audio data...');

while (~done) %loop to get data...note it will overflow the memory mapped file if you let it run too long
    
    t=tic;
    [audiodata,currentOffset,overflow,~] = PsychPortAudio('GetAudioData', pahandle); %populate the indices, note the types don't match....hmmmmm
    
    currentIndex=currentOffset+1; %it's not very useful in MATLAB if it starts at zero!?
    chunkSize=size(audiodata,2); %should be P.kNumSamples, but if not, the chunk should still get put in the right spot
    
    %vector of stamps
    timeStamps=linspace(currentIndex,currentIndex+chunkSize, chunkSize);
    
    %this is the data frame to write
    frame=vertcat(audiodata,timeStamps);
    
    %audioD(:,currentIndex:currentIndex+chunkSize-1) = audiodata; %stick the new data at the end of the old data
    audioD_memFile.data(1,1).d(:,currentIndex:currentIndex+chunkSize-1) = frame; %importantly, also stick it into the memory mapped file
    %display(['writing data into audio file at index ' int2str(currentIndex)]);
    i=i+1;
    while(toc(t)<(1/P.frameRate))
        %block for the rest of the frame duration
    end  
end


% Close the audio device:
PsychPortAudio('Close', pahandle);

ok=1;
