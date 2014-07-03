% hold off;
% function script_stepbystep_modded()

% clear all
% close all

lowFreq = 200;
highFreq=5000;
numChannel=64;
numCell=1;
binSize=100;
maxITD=1000;
degreePerClass=15;
score=zeros(16,9);

Fs = 44100;
nbits = 24;
acq_ch = 1; % acquisition channel
secs2record = 0.01;
device = 0; % 0 = default windows driver; 1 = usb mic; 3 = laptop mic
lineinput = audiorecorder(Fs, nbits, acq_ch, device);

overlapping = 0;
lastFrame = [];
isFirst = true;

lastSpikeResponse = zeros(numChannel,1);

useVideo = 1;
generateSpikesAudio = 1;

if (useVideo==1)
%     global vid;
    vid = videoinput('winvideo', 1);
    set(vid,'FramesPerTrigger',1);
    set(vid,'TriggerRepeat',Inf);
    triggerconfig(vid, 'Manual');
    start(vid);
end


while(1)
% loop = 1:10000

    recordblocking(lineinput, secs2record); % get audio from device
    
    if (useVideo==1) 
        trigger(vid); 
    
        IM = (getdata(vid,1,'uint8'));
        IMGray = rgb2gray(IM);
     
    
        IM_PREV = IMGray;
               
        IM_DIFF = abs(double(IMGray) - double(IM_PREV));         
    end
    
    
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
        plot( cf,  thisSpikeResponse/secs2record );
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
        
    if (useVideo == 1)
        subplot(2,3,5);
        imagesc(IM); 
        title('Current Video Frame')
    end   
end
% cleanup(vid);
 onCleanup(@()cleanup(vid));    
% end