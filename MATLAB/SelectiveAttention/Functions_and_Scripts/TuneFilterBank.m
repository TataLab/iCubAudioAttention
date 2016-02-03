function [ fWeights ] = TuneFilterBank( P )
%find spectral weights using envelope PCA
%

%do some setup.  This is redundant with the main loop, but it's better to
%modularize this code into a function here

display('learning target voice');
display('please introduce yourself to iCub and ask him to listen to you');
display('press a key when ready');
pause;



%prime the filterbank
P_inL=zeros(8,P.nBands);
P_inR=zeros(8,P.nBands);

%grab the current audio frame in the buffer, just to get the stamp of its
%last sample
frame=P.audioIn.Data(1,1).audioD(:,end-P.sizeFramePlusOverlap+1:end); %initialize the first frame 
tuningLastFrameStamp = frame(3,end);%ask what is the stamp of the most recent sample written (it's in the last column in the buffer)


tuningBuffer=zeros(4,P.frameDuration_samples*P.tuningDuration_frames); %stack up the frames



t=tic;
for i=1:P.tuningDuration_frames
    tuningBuffer=circshift(tuningBuffer,[0 -P.frameDuration_samples]);
    tuningBuffer(:,end-P.frameDuration_samples+1:end)=P.audioIn.Data(1,1).audioD(:,end-P.frameDuration_samples+1:end); %initialize the first frame 
    nextFrameStamp=tuningLastFrameStamp+P.frameDuration_samples; %increment
    tuningLastFrameStamp=nextFrameStamp;
    
    check=0;
    while(P.audioIn.Data(1,1).audioD(end-1,end)<nextFrameStamp)
        %spin until the next frame has been written into the buffer
        check=1;
    end
    if(~check)
        display('tuning is running too slow');
    end
end
toc(t);

%now filterbank and compute the envelopes 
[tuningFrameL,~,~,~,~]=gammatonePhase(tuningBuffer(1,:),P_inL,P.sampleRate, P.cfs);
[tuningFrameR,~,~,~,~]=gammatonePhase(tuningBuffer(2,:),P_inR,P.sampleRate, P.cfs);
    
%now compute the envelopes
tuningFrameEnvL=abs(hilbert(tuningFrameL));
tuningFrameEnvR=abs(hilbert(tuningFrameR));

%filter the envelopes to isolate the low-frequency AM envelope
%load('./Functions_and_Scripts/LowPass/LowPass20hz_iirChebI_order4_fs48000.mat');
load('./Functions_and_Scripts/BandPass5hz/BandPass5hz_chebI_order4_fs48000.mat');

tuningFrameEnvL=filtfilt(bp,tuningFrameEnvL);
tuningFrameEnvR=filtfilt(bp,tuningFrameEnvR);

%now find the weights of the filterbank based on common envelope
[fWeightsL,~,~,~,varL,~]=pca(tuningFrameEnvL);
[fWeightsR,~,~,~,varR,~]=pca(tuningFrameEnvR);

%we assume the target talker is the envelope with the most variance
%explained - thus the appropriate weights are the first column of fWeights.  These are the weights we'll apply to the filterbank on every
%frame
fWeightsL=fWeightsL(:,1);
fWeightsR=fWeightsR(:,1);

%pre-build a matrix of weights to efficiently scale the input frames
fWeightsL=repmat(fWeightsL,[1 P.frameDuration_samples]);
fWeightsR=repmat(fWeightsR,[1 P.frameDuration_samples]);
 
%the filterbank will output samples in columns so we'll arrange this matrix
%the same way to speed computation
fWeightsL=fWeightsL';
fWeightsR=fWeightsR';

%square the weights to make the vector more "peaky"
fWeightsL=fWeightsL.^2;
fWeightsR=fWeightsR.^2;

display(['found an envelope in the left channel that explains ' num2str(varL(1)) '% of signal variance']);
display(['found an envelope in the right channel that explains ' num2str(varR(1)) '% of signal variance']);

subplot(2,1,1);
bar(P.cfs,fWeightsL(1,:));
title('left squared PCA weights');
subplot(2,1,2);
bar(P.cfs,fWeightsR(1,:));
title('right squared PCA weights');

drawnow;

%return a concatenated matrix of weights with left first and right second
fWeights=zeros(2,P.frameDuration_samples,P.nBands);
fWeights(1,:,:)=fWeightsL;
fWeights(2,:,:)=fWeightsR;

display('Done learning.  Press a key to continue.');
pause;



end

