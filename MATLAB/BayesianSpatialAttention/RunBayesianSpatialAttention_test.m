%build a probabalistic map of space in radial beams and update it in a
%Bayesian way over time.
%Important insight: there can be more than one "map" of space.  There can
%be a "map" of probabilities for each sound object


display(['Running audioMapRadial using code at: ' mfilename('fullpath')]);
addpath('./Functions_and_Scripts');

%perform initialization and setup;  get back a struct with all the settings
%we'll need
P=ConfigureParameters;

currentMicHeading_degrees=0.0;  %the current heading of the robot
currentMicHeading_radians=0.0;

frame=P.audioIn.Data(1,1).audioD(:,end-P.sizeFramePlusOverlap+1:end); %initialize the first frame
lastFrameStamp = frame(3,end);%ask what is the stamp of the most recent sample written (it's in the last column in the buffer)

%prime the filterbank
P_inL=zeros(8,P.nBands);
P_inR=zeros(8,P.nBands);

%matrices for holding previous frames of rms amplitudes

pastAmp=ones(P.nPastFrames,P.nBands).*.0001; %we have to seed this with some arbitrarily small numbers
pastDeltaAmp=zeros(P.nPastFrames,P.nBands);


selectedBeam=1;
selectedAngle=1;

maxBeamsIndices=zeros(P.nBands,1);
frameCounter=1;

%set up an auditory object to keep track of sound sources
O.onsetTime=tic;
O.angle_micAligned=0.0;
O.angle_spaceAligned=0.0;
O.angle_spaceAligned_index=1;
O.salience=0.0;
O.radialPriors_initial_spaceAligned=ones(1,P.numSpaceAngles);
O.radialPriors_initial_micAligned=ones(1,P.numSpaceAngles);
O.radialPriors_updated_spaceAligned=ones(1,P.numSpaceAngles);
O.radialPriors_updated_micAligned=ones(1,P.numSpaceAngles);
O.radialSalience_updated_micAligned=zeros(1,P.numSpaceAngles);
O.radialSalience_updated_spaceAligned=zeros(1,P.numSpaceAngles);

%to not use top-down attention use uniform weights
O.fWeights=ones(P.nBands,P.frameDuration_samples); %note that this is bandsxsamples.  to apply these weights in the spectral stage we'll need samples by bands because of the filterbank output

%tune the filterbank by listening to a target talker
%[fWeights]=TuneFilterBank(P);

%for testing
% fWeightsL=ones(4096,64);
% fWeightsR=ones(4096,64);

%for testing
%load('mattWeights.mat');
numFramesToAcquire=100;
acquireProbData_micAligned=zeros(numFramesToAcquire,P.numSpaceAngles);
acquireProbData_spaceAligned=zeros(numFramesToAcquire,P.numSpaceAngles);
acquireSalienceData_micAligned=zeros(numFramesToAcquire,P.numSpaceAngles);
acquireSalienceData_spaceAligned=zeros(numFramesToAcquire,P.numSpaceAngles);



done=0;
while(~done)
    
    t=tic;
    
    
    %this reads output of the filterbank streamed by AudioCaptureFilterBank_YARP or
    %AudioCaptureFilterBank_PreRecorded
    frame=P.audioIn.Data(1,1).audioD(:,end-(P.frameDuration_samples+2*P.frameOverlap)+1:end); %note the overlap.  This is so that we can run beamformer and extract exactly frameDuration_samples from each frame by leaving off the tails.  Unnecessary unless you want to concatenate frames for output
    
    
    frameL=frame(1,:);
    frameR=frame(2,:);
    
    %%%%%%%    Pre-Attentive Stage    **********
    
    
    
    
    %%%%%%Spectral Pre-processing*****
    
    %decompose each channle using a gammatone filterbank
    %and stream out the filtered frames into two seperate files
    [fFrameL,P_outL,~,~,~]=gammatonePhase(frame(1,:),P_inL,P.sampleRate, P.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [fFrameR,P_outR,~,~,~]=gammatonePhase(frame(2,:),P_inR,P.sampleRate, P.cfs);
    P_inR=P_outR;
    
    %to use top-down attention apply the filterbank weights for the
    %selected target talker here (i.e. very early selection)
    %apply learned weights to the filterbank to select the target talker in
    %a top-down sense
    %     fFrameL=fFrameL.*O.fWeights';
    %     fFrameR=fFrameR.*O.fWeights';
    
    
    %compute the amplitude of each band
    amp_frameL=rms(fFrameL,1);
    amp_frameR=rms(fFrameR,1);
    
    amp=(amp_frameL+amp_frameR)./2;  %collapse left and right channels - assume they have (nearly) identical spectra
    
    %look at onsets and offsets independently
    deltaAmpOnsets=(amp-mean(pastAmp,1))./mean(pastAmp,1);  %subtract the mean of the past spectral amplitude and divide by the mean of the past spectral amplitude
    deltaAmpOffsets=(mean(pastAmp,1)-amp)./mean(pastAmp,1);
    pastAmp=circshift(pastAmp,[1 0]); %push the stack down and wrap
    pastAmp(1,:)=amp;  %overwrite the top of the stack
    
    
    deltaAmpOnsets(deltaAmpOnsets<0)=0; %only deal with increments
    deltaAmpOffsets(deltaAmpOffsets<0)=0;
    
    
    [onsetSpectralPeakValues,onsetSpectralPeakIndices]=findpeaks(deltaAmpOnsets); %find the peak values and their indices in the spectrum
    [offsetSpectralPeakValues,offsetSpectralPeakIndices]=findpeaks(deltaAmpOffsets); %find the peak values and their indices in the spectrum
    [frameSpectralPeakValues,frameSpectralPeakIndices]=findpeaks(amp); %find the absolute raw "peakiness" of the current frame
    
    %deal with the not-so-unlikely case of a single peak by just using the
    %height of the peak
    if(isempty(onsetSpectralPeakValues))
        [onsetSpectralPeakValues,onsetSpectralPeakIndices]=max(deltaAmpOnsets); %just use the single largest value
    end
    if(isempty(offsetSpectralPeakValues))
        [offsetSpectralPeakValues,offsetSpectralPeakIndices]=max(deltaAmpOffsets); %just use the single largest value
    end
    if(isempty(frameSpectralPeakValues))
        [frameSpectralPeakValues,frameSpectralPeakIndices]=max(amp); %just use the single largest value
    end
    
    onsetAudioSalience= sum(onsetSpectralPeakValues) * length(onsetSpectralPeakValues); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene
    offsetAudioSalience= sum(offsetSpectralPeakValues) / length(offsetSpectralPeakValues); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene
    
    frameSalience=sum(frameSpectralPeakValues) * length(frameSpectralPeakValues);
    %%inspection
    
    
    %%%end spectral salience
    
    fFrameL=fFrameL';
    fFrameR=fFrameR';
    
    
    %%%%%%  Selective Attention Stage *********
    
    
    %compute the time-decaying salience
    O.tdSalience = (2 * toc(O.onsetTime) * exp(-toc(O.onsetTime) * 0.8) + exp(-toc(O.onsetTime) * 0.8)) * O.salience;
    plot(frameCounter,O.tdSalience,'ro');
    drawnow;
    hold on;
    
    %compute the difference between space and mic angles in units of indices
    %into the lookup vectors
    currentMicHeading_index=floor(currentMicHeading_degrees/P.radialResolution_degrees);  %we need to convert heading in degrees into the index of that in the vector of angles.the midline is a zero
    %display(currentMicHeading_index);

    %check if a new object appeared
    if(onsetAudioSalience>O.tdSalience && onsetAudioSalience > P.attentionCaptureThreshold)
        %a new object captured attention so update all the object features
        O.salience=onsetAudioSalience;  %the current objects salience
        O.onsetTime=tic;%take the last time stamp of the frame to be the onset time ... note that's arbitrarily inaccurate to within frameDuration
        
        %set new filterbank weights, or set ones to not use spectral
        %attention
        O.fWeights=repmat(deltaAmpOnsets',[1 P.frameDuration_samples]);
        %O.fWeights=ones(P.nBands,P.frameDuration_samples);
        
        %get some variables ready for the output of the beamformer stage
        thisFrameImage=zeros(P.nBands,P.nBeams,P.frameDuration_samples+2*P.frameOverlap); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)
        
        %sweep a beam to find a first guess at this new object's location
        %   %this is exactly a bank of delay-and-sum beamformers
        for bandCounter=1:P.nBands
            
            thisBandL=fFrameL(bandCounter,:);
            thisBandR=fFrameR(bandCounter,:);
            
            beamCounter=1;
            for b=-P.nBeamsPerHemifield:P.nBeamsPerHemifield
                tempR=circshift(thisBandR,[0 b]); %shift through each lag
                thisFrameImage(bandCounter,beamCounter,:)=thisBandL+tempR; %add the shifted right channnel to the unshifted left channel
                beamCounter=beamCounter+1;
                
            end
            
        end
        
        %localize by find the beam with the most energy
        %note that the filterbank weights multiply through this computation
        
        thisFrameRMS=rms(thisFrameImage,3); %find the bandsxbeams matrix of rms values
        
        %this computes a first-guess angle to steer towards
        weightedRMS=sum(thisFrameRMS,1);
        [maxRMS,maxBeam]=max(weightedRMS,[],2);
        
        O.angle_micAligned=P.angles(maxBeam);
        %display(O.angle_micAligned*180/pi);
        
        O.angle_spaceAligned=O.angle_micAligned-currentMicHeading_radians;
        
        %to initialize the prior probabilities of each angle in the object's
        %probabalistic map, we need to normalize the initial beams
        %initialize the map of prior probabilities for this object
        
        
        %%this will use a single vector of priors for every frequency.  If we
        %%arrive at that vector by pooling across frequencies, then this makes
        %%sense
        initialPriors=weightedRMS;
        reflectedPriors=fliplr(initialPriors);
        surroundPriors=[initialPriors reflectedPriors(:,2:end-1)]; %reflect the front onto the back (because we've got only two mics in the array
        surroundPriors=circshift(surroundPriors,[0 P.nBeamsPerHemifield]); %we need this lined up with P.micAngles which has -180 as its first element
        
        %interpolate around the angles
        interpolatedSurroundPriors=interp1(P.micAngles,surroundPriors,P.spaceAngles,'spline'); %the beam distribution isn't linearly arranged around the circle and doesn't sample the space with the same resolution as the angles that point into external space.  Interpolate.
        
        %normalize
        normInterpolatedSurroundPriors=interpolatedSurroundPriors./sum(interpolatedSurroundPriors);
        
        %map priors and align
        O.radialPriors_initial_micAligned=interpolatedSurroundPriors;  %reset or add old priors .... depends on what you want .. have to think about this
        O.radialPriors_initial_micAligned=O.radialPriors_initial_micAligned./sum(O.radialPriors_initial_micAligned);
        O.radialPriors_initial_spaceAligned=circshift(O.radialPriors_initial_micAligned,[0 currentMicHeading_index]); %circshift it into real-world space so the mic angle is decoupled from the world around it
        
        %initialize the updated priors
        O.radialPriors_updated_micAligned=O.radialPriors_initial_micAligned;
        O.radialPriors_updated_spaceAligned=O.radialPriors_initial_spaceAligned;
        
        %initialize the salience map
        O.radialSalience_updated_micAligned=O.radialPriors_initial_micAligned.*O.tdSalience;
        O.radialSalience_updated_spaceAligned=O.radialPriors_initial_spaceAligned.*O.tdSalience;

        
        if(P.sendAngleToYarp==1)
            
            audioAttentionControl('/mosaic/angle:i',O.angle_micAligned*180/pi,1);
            
            %%here we need to get the angle of the microphones relative to
            %%the space around the robot.  On red iCub we need to replace
            %%this with some code that actually reads the head angle
            currentMicHeading_degrees=currentMicHeading_degrees+O.angle_micAligned*180/pi;
            
            %jump some hoops to make sure we know where the head is
            %pointing
            if(currentMicHeading_degrees<-40.0)
                currentMicHeading_degrees=-40.0;
            elseif(currentMicHeading_degrees>40.0)
                currentMicHeading_degrees=40.0;
            end
            
            currentMicHeading_radians=currentMicHeading_degrees*pi/180;
            
        end
        
        display(['New object at ' num2str(O.angle_spaceAligned*180/pi)]);
        %display(currentMicHeading_degrees);
        
      
        %%%else update!%%%%%
    else %don't update unless some time has past
        
        %previous filterbank weights for the selected talker to the beamforming
        %stage, use the weights stored in the object
        fFrameL=O.fWeights.*fFrameL;
        fFrameR=O.fWeights.*fFrameR;
        
        
        %get some variables ready for the output of the beamformer stage
        thisFrameImage=zeros(P.nBands,P.nBeams,P.frameDuration_samples+2*P.frameOverlap); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)
        
        %sweep a beam to find the most recent evidence angle
        for bandCounter=1:P.nBands
            
            thisBandL=fFrameL(bandCounter,:);
            thisBandR=fFrameR(bandCounter,:);
            
            beamCounter=1;
            for b=-P.nBeamsPerHemifield:P.nBeamsPerHemifield
                tempR=circshift(thisBandR,[0 b]); %shift through each lag
                thisFrameImage(bandCounter,beamCounter,:)=thisBandL+tempR; %add the shifted right channnel to the unshifted left channel
                beamCounter=beamCounter+1;
                
            end
            
        end
        
        %localize by find the beam with the most energy
        %note that the filterbank weights multiply through this computation
        
        thisFrameRMS=rms(thisFrameImage,3); %find the bandsxbeams matrix of rms values
        
        %this computes a first-guess angle to steer towards
        weightedRMS=sum(thisFrameRMS,1);
        [maxRMS,maxBeam]=max(weightedRMS,[],2);
        
        
        %pass the object with its vector of priors in external space and the current angle
        %updatePriors will rotate the priors to align with mic space and use these to update the priors using Bayes and return
        %the object with updated priors (i.e. posteriors which can be used
        %as priors in the subsequent iteration)
        [O.radialPriors_updated_micAligned,O.radialPriors_updated_spaceAligned]=UpdatePriors(O,maxBeam,currentMicHeading_index,P,O.tdSalience);
        
        O.radialSalience_updated_micAligned=O.radialPriors_updated_micAligned.*frameSalience;
        O.radialSalience_updated_spaceAligned=O.radialPriors_updated_spaceAligned.*frameSalience;

        %find the new peak in the updated priors within a field of view
% 
%         [~,offsetIndex]=max(O.radialPriors_updated_micAligned(P.fov_indices));
%         offsetError_degrees=P.fov_angles(offsetIndex)*180/pi;
%                 
%         %compute the offset between where we're looking and where is the
%         %new peak
%         %O.angle_micAligned=currentMicHeading_degrees-offsetError_degrees;
%         
%         %
%         if(P.sendAngleToYarp==1)
%             
%             %here we send a vector of 360 degrees to
%             %audioAttentionControl('/mosaic/salienceImage:i',O.radialPriors_spaceAligned*180/pi,1);
%             audioAttentionControl('/mosaic/angle:i',offsetError_degrees,1);
%             
%             %update where we think the head is pointing, a bit circular
%             currentMicHeading_degrees=currentMicHeading_degrees+offsetError_degrees;
%             
%             if(currentMicHeading_degrees<-40.0)
%                 currentMicHeading_degrees=-40.0;
%             elseif(currentMicHeading_degrees>40.0)
%                 currentMicHeading_degrees=40.0;
%             end
%             
%             currentMicHeading_radians=currentMicHeading_degrees*pi/180;
%         end
%     
    
    end
    
    
    
    if(frameCounter<=numFramesToAcquire)
        display('acquiring a frame');
        acquireProbData_spaceAligned(frameCounter,:)=O.radialPriors_updated_spaceAligned;
        acquireProbData_micAligned(frameCounter,:)=O.radialPriors_updated_micAligned;
        acquireSalienceData_micAligned(frameCounter,:)=O.radialSalience_updated_micAligned;
        acquireSalienceData_spaceAligned(frameCounter,:)=O.radialSalience_updated_spaceAligned;
    else
        display('not acquiring a frame');
    end
    
%     plot(P.spaceAngles*180/pi,O.radialPriors_initial_spaceAligned*180/pi,'ro');
%     hold on;
%     plot(P.spaceAngles*180/pi,O.radialPriors_updated_spaceAligned*180/pi,'go');
%     %line([O.angle_spaceAligned*180/pi O.angle_spaceAligned*180/pi],[0 .1]);
%     hold off;
%     drawnow;

%build a frame image
    %micAlignedImage=repmat(O.radialPriors_updated_micAligned(P.fov_indices),[180 1]);
    %imagesc(P.spaceAngles(P.fov_indices)*180/pi,1:length(P.fov_indices),micAlignedImage);
%     plot(O.radialSalience_updated_spaceAligned);
%     ylim([0.0 2.0]);
%     drawnow;
    
    if(P.useDesktopRobot==1)
        P.motorControl=TurnDegrees(P.motorControl,O.angle_mic);
        display(['current heading is at ' num2str(currentMicHeading_degrees)]);
        display(['turning robot ' num2str(O.angle_mic) ' degrees']);
        %pause(.5);
    end
    
    
    %     bar(O.radialPriors.*O.salience);
    %     ylim([0 100]);
    %     drawnow;
    
    
    %on every frame we'll write the object struct into an array
    %so we can inspect it later
    
    
    %increment for next frame
    nextFrameStamp=lastFrameStamp+P.frameDuration_samples; %increment
    lastFrameStamp=nextFrameStamp;
    frameCounter=frameCounter+1;
    
    while(P.audioIn.Data(1,1).audioD(end-1,end)<nextFrameStamp)
        %spin until the next frame has been written into the buffer
    end
    
    
    
    % toc(t);
end