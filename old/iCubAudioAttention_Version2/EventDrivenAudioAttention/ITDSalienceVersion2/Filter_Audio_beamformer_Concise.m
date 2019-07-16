
[yy,SamplingFre]=readwav('./audioTest_right_to_left.wav');
yy=yy'; %sound vectors go left to right

Steps=2^12;
Steps_seconds=Steps/SamplingFre;
D=.14;
C=340.5;
 
frameCounter=0; %keep track of the 
done=0; %just keep looping
while(~done)
    
    
    t=tic();
    
%     %FrameL and frameR are left and right channel pure audio signal in
%     %integrating the code for icub just use the recorded(captured) audio frame from icub
%     frameL=yy(iii*Steps+1:(iii+1)*Steps,1);
%     frameR=yy(iii*Steps+1:(iii+1)*Steps,2);
    
    
    [frameL,frameR,yy]=GetCurrentAudioFrame_prerecorded(yy,Steps);
    frameCounter=frameCounter+1; %increment the number of frames we've grabbed
    
    %Time is the collapsed time from beginning of running the algorithm
    Time=(frameCounter*Steps+1:(frameCounter+1)*Steps)/SamplingFre;
    
    %Time0 is the time of one frame starting from zero
    Time0=(0*Steps+1:(1)*Steps)/SamplingFre;
    
   %%% Design GammatoneFilter between low_cf and high_cf
    low_cf=500;high_cf=5000;numchans=20;
    % center frequencies based on Erb scale
    %YOU should  add the gammatoneFast folder to matlab path 
    cfs = MakeErbCFs(low_cf,high_cf,numchans);

    % get the signal out of filter also its envelope and the delay due to the
    % filter. 'align' basically aligns the delay across the filter bank 
    fs=SamplingFre;
    align=true;
    [GF_frameL,envL,delayL] = gammatoneFast(frameL',cfs,fs,align) ; %note the transpose due to Mo's gamma filter bank needing column vectors
    [GF_frameR,envR,delayR] = gammatoneFast(frameR',cfs,fs,align) ;
    
    
    imagesc(GF_frameL);
    drawnow;
        
    
    %%%% Start of beamforming from 10 to 170 degree
    numElements=2;%Number of Mics
    spacing=D;%space between Mics
    
    speedSound = C;  % m/s
    
    ANGLE_RESOLUTION=360;    % Number of angle ranges that beamformer should go through them % This means the beams moves from zero to 180 by 1 degree difference. so the spatial resolution of the beams are only 1 degree.
    startAngle_index=10*(ANGLE_RESOLUTION/180); % the prepherial angles zero to 10 degrees and 170 to 180 degrees are ignored. It means that beams dont sweep these two area. This resolution in space is done to increase the speed of the algorithm
    endAngle_index=170*(ANGLE_RESOLUTION/180);
    
%     SteeringAngle is composed of angles from 10 to 170 degree by resolution of 1 degree
    SteeringAngle =  180.0 * (startAngle_index:endAngle_index) / (ANGLE_RESOLUTION-1);
    SteeringAngle=SteeringAngle*pi/180;%SteeringAngle in degree
    LS=length(SteeringAngle); %number of beams
    
    % initializing some vectors to record the results in beam sweeping iteration 
    logOutput=zeros(numchans,LS);
    Output_Index=zeros(numchans,LS);
    Index_MAX_P_L=zeros(numchans,1);
    
    
    
    %%%Calculating energy of each channel.
    EnergyL=trapz(Time0,abs(GF_frameL.^2));
    
    %%%Compose the Signal of beamformer here
    Fframe={GF_frameL, GF_frameR}; % using the output of gammatone signal
    %%% end of composing the signal
    
    
    %making the delay(time) and sample_delay vector for beamforming
    delay = spacing * (-cos(SteeringAngle)) / speedSound;%to compensate for the delay in the left and right channel
    sample_delay=round(delay*SamplingFre);
        
    %%beamformer; sweeping beams from 10 to 170 degree for each filter channel(numchans times).
    for jj=1:numchans
        
        for j=1:LS %Steer the beam (Ls is the number of beams)
            
            if j>1 %these two if conditions avoid repetitive computation for beams with the same sapmle_delay and reduce the calculation time
                if sample_delay(j)~=sample_delay(j-1)
                    
                    %the summation process of the left and right channel 
                if (sample_delay(j)>0)
                       OutSum=Fframe{1}(:,jj)+[zeros(abs(sample_delay(j)),1)  ;Fframe{2}(1:end-abs(sample_delay(j)),jj)];
                else
                       OutSum=Fframe{1}(:,jj)+[Fframe{2}(abs(sample_delay(j))+1:end,jj) ; zeros(abs(sample_delay(j)),1)];
                end
                    
                    %normalize the output signal of beamformer by number
                    %of channels
                    OutSum = OutSum / numElements;
                    
                    %storing the peak of the all beams 
                   Abs_OutSum=abs(OutSum);                  
                    MAX_OutSum=max(Abs_OutSum);
                    logOutput(jj,j) = (MAX_OutSum); %jj is the filterbank index, j is the index of spatial beams
                    
                else
                    % if the sample delay is the same just store the same
                    % previous output for the current beam
                    
                    Abs_OutSum=abs(OutSum);
                    MAX_OutSum=max(Abs_OutSum);
                    logOutput(jj,j) = (MAX_OutSum);
                    
                end
            else
                % For the first beam (j==1), initialization of the process
                if (sample_delay(j)>0)
                       OutSum=Fframe{1}(:,jj)+[zeros(abs(sample_delay(j)),1)  ;Fframe{2}(1:end-abs(sample_delay(j)),jj)];
                else
                       OutSum=Fframe{1}(:,jj)+[Fframe{2}(abs(sample_delay(j))+1:end,jj) ; zeros(abs(sample_delay(j)),1)];
                end
                
                %normalize the output signal of beamformer by number of channels                
                OutSum = OutSum / numElements;
                [MAX_OutSum, ~]=max(abs(OutSum));
                              
                
                logOutput(jj,j) = (MAX_OutSum);%jj is the filterbank index, j is the index of spatial beams
                
            end %j>1
        end%SteeringAngle
    end %num of bins(filters)
    
%     imagesc(Output_Index);
%     drawnow;
    
    
    [DOA_Level ,Index_DOA]=max(logOutput,[],2);% find the beam with maximum peak for each channel of filter
    DOA=SteeringAngle(Index_DOA)*180/pi-90; % convert it to degree and shift by 90 degree so DOA(direction of arrival) lies between -90 to 90 degree
    currentAngle_temp=DOA;

    
   %%%Now start sorting bins and weighting them to choose an angle out of all channels.
    %    It looks like a night mare code I will clean it later. From here to the end 
    %    For integrating the code you dont need to touch this part any more just copy and paste
    

    [M, F] = mode(currentAngle_temp(1,:));
    majority_base=M;
  
    VALUE=[];
    NUM_VAL=[];
    input0=currentAngle_temp(1,:);
 %  input0=round(input0,2);
    input0=round(input0*100)/100;
    [sorted_tem,sortingIndices_tem] = sort(input0,'descend');
    [Value,Num_Val]=mode(sorted_tem); counter=0;
    
    while Num_Val~=0
    counter=counter+1;
    VALUE(counter)=Value;
    NUM_VAL(counter)=Num_Val;
    ibcc=find(sorted_tem==Value);
    ibc=ibcc(1,1);
    sorttemp=[sorted_tem(1,1:ibc-1), sorted_tem(1,(ibc+Num_Val):end)];
    clear sorted_tem
    sorted_tem=sorttemp;
    clear sorttemp
   [Value,Num_Val]=mode(sorted_tem);   
    end
bins=-80:10:80;
bin_arrange=sign(VALUE).*floor((abs(VALUE)+5)/10);
for kk=-9+10:9+10
idx=find(bin_arrange==(kk-10));
if ~isempty(idx)
merged_bin_value(kk)=sum(NUM_VAL(idx).*VALUE(idx))/sum(NUM_VAL(idx));
merged_bin_level(kk)=sum(NUM_VAL(idx));
else
merged_bin_value(kk)=100;
merged_bin_level(kk)=0;
end
end
tag=0;
for kk=-9+10:9+10
    if ((tag==0)&&(merged_bin_value(kk)~=100))
   
        if (kk>1)&&(kk<19)
        value_vector=([merged_bin_value(kk-1)  merged_bin_value(kk) merged_bin_value(kk+1)]);
        level_vector=[merged_bin_level(kk-1)  merged_bin_level(kk) merged_bin_level(kk+1)];
        
        else
            
            if(kk==1)
        value_vector=([ merged_bin_value(kk) merged_bin_value(kk+1)]);
        level_vector=[ merged_bin_level(kk) merged_bin_level(kk+1)];
            elseif (kk==19)
        value_vector=([merged_bin_value(kk-1)  merged_bin_value(kk) ]);
        level_vector=[merged_bin_level(kk-1)  merged_bin_level(kk) ];             
                
            end
            
           
        end
        combined=sum(value_vector .* level_vector)/sum(level_vector);
  
    
    if ( combined>((kk-10)*10-5))&&(combined<=((kk-10)*10+5))
       tag=1;
     merged2_bin_value(kk)=combined;
     merged2_bin_level(kk)=sum(level_vector);
    else
      merged2_bin_value(kk)=100;
        
      merged2_bin_level(kk)=0;
          
        
    end
    else
        if tag==1
        merged2_bin_value(kk)=100;
        
        merged2_bin_level(kk)=0;    
        else
        merged2_bin_value(kk)=merged_bin_value(kk);
        
        merged2_bin_level(kk)=merged_bin_level(kk);
        end
        tag=0;
    end
end
 
 

 
 
    clear sorted_tem
    currentAngle_temp(2,:)=EnergyL;
    [sortedX,sortingIndices] = sort(currentAngle_temp(2,:),'descend');
    [MM,MF]=mode(currentAngle_temp(1,sortingIndices(1:20)));
    
    %%%THIS IS THAT AWSOME ANGLE. HERE YOU ARE SEND IT TO ROBOT!!
    majority_base_majorityEnergy=MM;
%    display(MM);
    
    
    display(['processing a ' num2str(Steps_seconds) ' frame took ' num2str(toc(t)-Steps_seconds) ' seconds longer than real-time']);
%     [xpeak,ypeak]=pol2cart(MM*pi/180,1);
%     compass(xpeak,ypeak,'r');
%     drawnow;


    
end %simulation loop