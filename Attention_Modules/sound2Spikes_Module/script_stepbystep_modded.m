% hold off;
% function script_stepbystep_modded()

clear all
close all

useVideo = 0; % set 1 to use webcam
generateSpikesAudio = 1; % set 1 to generate spikes from filtered audio signal
useAudioDevice = 0; % set 1 to use audio device (mic)
                    % will ask for a wav file if set to 0

lowFreq = 200; % lower frequency bound for filtering of input audio signal
highFreq=5000; % Upper frequency bound for filtering input audio.
numChannel=64; % Number of band to divide the above frequency range in.
numCell=1; % Additional feature: Sets number of neurons that spike for the above frequency bands but for increasing ampltude. Default and recommended for now is 1;
binSize=100; % Number of TIME bins to divide the input signal
maxITD=1000; % Additional variable: For use with spike based localizer. Not yet used.

% Parameters for the input audio device
Fs = 44100; % Frequency of acquisition. Set 44100 for intel/realtek based audio drivers. Set to 48000 for iCub.
nbits = 16; % Resolution of input samples. Use highest possible by your device to avoid clipping.
acq_ch = 1; % Number of microphones used. Default 1 for laptop. Set to 2 when using with iCub or stereo recordings.
secs2record = 0.02; % Length of each frame of audio in seconds.

if(useAudioDevice)
% If useAudioDevice = 0 then set which device you are using. Will add
% another number to add portaudio/iCub.
device = 0; % 0 = default windows driver; 1 = usb mic; 3 = laptop mic
lineinput = audiorecorder(Fs, nbits, acq_ch, device);

overlapping = 0;
lastFrame = [];
isFirst = true;
end

lastSpikeResponse = zeros(numChannel,1);



if (useVideo==1)
%     global vid;
    vid = videoinput('winvideo', 1);
    set(vid,'FramesPerTrigger',1);
    set(vid,'TriggerRepeat',Inf);
    triggerconfig(vid, 'Manual');
    start(vid);
end

processData = true;
while(processData)
% loop = 1:10000

    if (useVideo==1) 
        trigger(vid); 
        IM = (getdata(vid,1,'uint8'));
        IMGray = rgb2gray(IM);
        IM_PREV = IMGray;
        IM_DIFF = abs(double(IMGray) - double(IM_PREV));         
    else
        IM = imread('no_video.jpg');
    end
    
    if(useAudioDevice)
        recordblocking(lineinput, secs2record); % get audio from device
    
        if(isFirst == true)
            temp = getaudiodata(lineinput);
            lastFrame = temp;
            data = lastFrame;
            isFirst = false;
        else
            temp = getaudiodata(lineinput);
            data = [lastFrame(end-round(numel(lastFrame)*overlapping):end); temp(1:round(numel(temp)*(1-overlapping)))];
            lastFrame = temp;          
        end
    else
        % ask user to specify wav file.
        [fileName, pathName] = uigetfile('.wav');
        disp(pathName);
        if([pathName fileName] == 0)
            return;
        end
        data = wavread([pathName fileName]);
        data = data(:, 1);
        processData = false;
    end
    
%     data = (data - mean(data))/max(data);
    [tap, cochwave,fs,cf ] = cochlea_modded(data, Fs, numChannel, lowFreq, highFreq );   

%     hold on;
%     for i=1:numChannel
%         x = cochwave{1,1}(17-i,1:1200)+0.01*i;
% %     plot(cochwave{1,2}(17-i,1:1200)+0.01*i,'green');    
%     end

%     xlabel('Time (ms)');
%     ylabel('Frequency Channel');
%     set(gca,'XTick',1:88:1200,'XTickLabel',0:2:30);
%     set(gca,'YTick',0:0.01:16*0.01,'YTickLabel',0:16);
    if(generateSpikesAudio == 1)
        [spikes,aerdata] = spikegenerator_modded( cochwave,fs,numChannel,numCell);
        spikes{1,1} = spikes{1,1}(:,200:end);
    end
    
    figure(2);
    title('Spike based response of the input audio-video signal');

    subplot(2,3,1); 
    plot(data); 
    xlabel('Time (ms)');
    title('Audio signal');

    subplot(2,3,2); 
    imagesc(cochwave{1,1}(:,200:end));
    set(gca, 'YDir', 'normal');
%     set(gca, 'YTick', cf(end):cf(1));
    xlabel('Time (ms)');
    title('Time-Frequency plot');

    %   
    if(generateSpikesAudio == 1)
        thisSpikeResponse = sum(spikes{1,1},2);
        differenceSpikeResponse = thisSpikeResponse - lastSpikeResponse;
        subplot(2,3,4); 
        plot( cf,  thisSpikeResponse);
        axis([cf(end) cf(1) 0 5000]);
        lastSpikeResponse = thisSpikeResponse;
        title('Frequency dependent spike count');
        xlabel('Frequency Channel');
        
        thisSpikeTimes = sum(spikes{1,1},1);
        subplot(2,3,3); 
        plot( thisSpikeTimes );
        title('Time dependent spike count');
        xlabel('Time');
    
        subplot(2,3,6); imagesc(spikes{1,1});
        colormap('summer');
        xlabel('Time (ms)');
        ylabel('Frequency Channel');
        set(gca, 'YDir', 'normal');
        title('Raster plot');

    end
        
%     if (useVideo == 1)
        subplot(2,3,5);
        imagesc(IM); 
        title('Current Video Frame')
%     end   
end
% cleanup(vid);
 onCleanup(@()cleanup(vid));    
% end