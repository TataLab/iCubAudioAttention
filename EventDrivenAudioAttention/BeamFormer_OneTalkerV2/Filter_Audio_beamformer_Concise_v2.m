%  [yy,SamplingFre]=readwav('C:\Users\Home\Desktop\audioTest_right_to_left_with_distractor.wav');
%[yy,SamplingFre]=readwav('./audioTest_right_to_left.wav');


Steps=SamplingFre*.25; %steps is duration of frame in samples
spacing=.14;%space between Mics
speedSound=340.5;% m/s
numElements=2;%Number of Mics


%%% Design GammatoneFilter between low_cf and high_cf
low_cf=500;high_cf=4000;numchans=15;
% center frequencies based on Erb scale
%YOU should  add the gammatoneFast folder to matlab path
cfs = MakeErbCFs(low_cf,high_cf,numchans);


ANGLE_RESOLUTION=36*4; %360   % Number of angle ranges that beamformer should go through them % This means the beams moves from zero to 180 by 1 degree difference. so the spatial resolution of the beams are only 1 degree.
startAngle_index=10*(ANGLE_RESOLUTION/180); % the prepherial angles zero to 10 degrees and 170 to 180 degrees are ignored. It means that beams dont sweep these two area. This redustion in space is done to increase the speed of the algorithm
endAngle_index=170*(ANGLE_RESOLUTION/180);
%Time0 is the time of one frame starting from zero
Time0=(0*Steps+1:(1)*Steps)/SamplingFre;

align=true;

% SteeringAngle is composed of angles from 10 to 170 degree by resolution of 360/ANGLE_RESOLUTION degree
SteeringAngle =  180.0 * (startAngle_index:endAngle_index) / (ANGLE_RESOLUTION-1);
SteeringAngle=SteeringAngle*pi/180;%SteeringAngle in degree
LS=length(SteeringAngle); %number of beams

%making the delay(time) and sample_delay vector for beamforming
delay = spacing * (-cos(SteeringAngle)) / speedSound;%to compensate for the delay in the left and right channel
sample_delay=round(delay*SamplingFre);
% initializing some vectors to record the results in beam sweeping itiration
logOutput=zeros(numchans,LS);

%     If there is any prior knowledge about number of talker indicate it here.
Number_of_possible_talker=1;% 1 or maximum 2 if you are not certain choose 2

Vector_of_talkers=zeros(Number_of_possible_talker,20);
Vector_of_talkers_weight=zeros(Number_of_possible_talker,20); %could be the possibilty value or just 0 or 1
var_vector=zeros(Number_of_possible_talker,20);
tag_new_angle=0; %should go top
past_Output_angle=0;
Energy_threshold=10^-4;
for iii=10:floor(length(yy(:,1))/Steps)
 
    t=tic(); %grab time to see if we're lagging behind real time
    
    %FrameL and frameR are left and right channel pure audio signal in
    %integrating the code for icub just use the recorded(captured) audio frame from icub
    frameL=yy(iii*Steps+1:(iii+1)*Steps,1);
    frameR=yy(iii*Steps+1:(iii+1)*Steps,2);
    
    EnergyL_Total=trapz(Time0,abs(frameL.^2));
    EnergyR_Total=trapz(Time0,abs(frameR.^2));
    Energy=(EnergyL_Total+EnergyR_Total)/2;
    %Time is the collapsed time from beginning of running the algorithm
    %     Time=(iii*Steps+1:(iii+1)*Steps)/SamplingFre;
    player = audioplayer(yy(iii*Steps+1:(iii+1)*Steps,:), SamplingFre);
    play(player);
    
    
    
    % get the signal out of filter also its envelope and the delay due to the
    % filter. 'align' basically aligns the delay across the filter bank
    
    
    %
    [GF_frameL,~,~] = gammatoneFast(frameL,cfs,SamplingFre,align) ;
    [GF_frameR,~,~] = gammatoneFast(frameR,cfs,SamplingFre,align) ;
    
    %%%% Start of beamforming from 10 to 170 degree
   
    
    %%%Calculating energy of each channel.
    EnergyL=trapz(Time0,abs(GF_frameL.^2));
    
    %%%Compose the Signal of beamformer here
    Fframe={GF_frameL, GF_frameR}; % using the output of gammatone signal
    %%% end of composing the signal
  
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
                    [MAX_OutSum, ~]=max(abs(OutSum));
                    
                    
                    
                    logOutput(jj,j) = (MAX_OutSum); %jj is the filterbank index, j is the index of spatial beams
                    
                else
                    % if the sample delay is the same just store the same
                    % previous output for the current beam
                    
                    [MAX_OutSum, ~]=max(abs(OutSum));
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
    [~ ,Index_DOA]=max(logOutput,[],2);% find the beam with maximum peak for each channel of filter
    DOA=SteeringAngle(Index_DOA)*180/pi-90; % convert it to degree and shift by 90 degree so DOA(direction of arival) lies between -90 to 90 degree
    currentAngle_temp(1,:)=DOA;
    
   
    %%%Now start sorting bins and weighting them to choose an angle out of all channels.
    %    It looks like a night mare code I will clean it later. From here to the end
    %    For integrating the code you dont need to touch this part any more just copy and paste
    
    %     [M, ~] = mode(currentAngle_temp(1,:));
    %     majority_base=M;
    
    
    VALUE=[];
    NUM_VAL=[];
    input0=currentAngle_temp(1,:);
    input0=round(input0*100)/100;
    [sorted_tem,sortingIndices_tem] = sort(input0,'descend');
    [Value,Num_Val]=mode(sorted_tem); counter=0;
    % This step just find the angle with highest majority in filterbank and
    % number of its frequency
    
    %     Now lets fill the VALUE and NUM_VAL by in row from highest frequent one to the lower one.
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
    
    
    
    %Now lets get weight to spatial bins (in degree) based on its majority in filterbank. It collects(merge) any angle in the range of 10 degrees in each bins
    bins=-80:10:80; %totaly 19 bins
    bin_arrange=sign(VALUE).*floor((abs(VALUE)+5)/10);
    for kk=-9+10:9+10 %totaly 19 bins
        idx=find(bin_arrange==(kk-10));
        if ~isempty(idx)
            merged_bin_value(kk)=sum(NUM_VAL(idx).*VALUE(idx))/sum(NUM_VAL(idx));
            merged_bin_level(kk)=sum(NUM_VAL(idx));
        else
            merged_bin_value(kk)=100; % get the value of 100 degree to bins without any angle
            merged_bin_level(kk)=0;
        end
    end
    
    %     Now merging neighbering bins to a single bin
    tag=0;
    for kk=-9+10:9+10
        if ((tag==0)&&(merged_bin_value(kk)~=100))
            
            if (kk>1)&&(kk<19) %bins in the middle
                value_vector= [merged_bin_value(kk-1)  merged_bin_value(kk) merged_bin_value(kk+1)];
                level_vector= [merged_bin_level(kk-1)  merged_bin_level(kk) merged_bin_level(kk+1)];
                
            else
                
                if(kk==1) % first bin
                    value_vector=([ merged_bin_value(kk) merged_bin_value(kk+1)]);
                    level_vector=[ merged_bin_level(kk) merged_bin_level(kk+1)];
                elseif (kk==19) % terminal bin
                    value_vector=([merged_bin_value(kk-1)  merged_bin_value(kk) ]);
                    level_vector=[merged_bin_level(kk-1)  merged_bin_level(kk) ];
                    
                end
                
                
            end
            combined=sum(value_vector .* level_vector)/sum(level_vector);
            
            %Now sort bins based on new combination of bins
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
    [MM,~]=mode(currentAngle_temp(1,sortingIndices(:)));
    
    %%%THIS IS THAT AWSOME ANGLE. HERE YOU ARE SEND IT TO ROBOT!!
    majority_base_majorityEnergy=MM;
    
    display(['frame duration: ' num2str(Steps/SamplingFre) ' took ' num2str(toc(t))]); 
    
    %%%%%%%
    %plot
    [xpeak,ypeak]=pol2cart(majority_base_majorityEnergy*pi/180,1);
    compass(xpeak,ypeak,'r');
    
    drawnow;
    %done plotting
    %%%%%%%%%
    
end






    
%     %finding two talker 
%   
%     [A_level,A_idx]=max(merged2_bin_level);
%     merged2_bin_level_secondTalker=merged2_bin_level;
%     merged2_bin_level_secondTalker(A_idx)=0;
%     [A2_level,A2_idx]=max(merged2_bin_level_secondTalker);
%     angle=merged2_bin_value(A_idx);
%     
%  
%     
%     
%     Vector_of_talkers= shiftc(Vector_of_talkers);
%     Vector_of_talkers_weight= shiftc(Vector_of_talkers_weight);
%     var_vector= shiftc(var_vector);
%     
%     
%     
%     %First talker vector
%     if (A_level/numchans>0.5)&&(Energy>Energy_threshold)
%         
%         Vector_of_talkers(1,1)= angle;
%         Vector_of_talkers_weight(1,1)=1;
%         
% %         display(['First talker with probability of more than 50 percent, P>50% is at ', num2str(angle),' degree'] )
%     else
%         Vector_of_talkers_weight(1,1)=0;
% %         display(['First talker with probability of more than 50 percent, P<50% is at ', num2str(angle) ,' degree, possibility of ambiguity'])
%         
%     end
% %     var1=sqrt(var(Vector_of_talkers(1,1:3).*Vector_of_talkers_weight(1,1:3)));
%     var1=sqrt(   var(    Vector_of_talkers(1,logical(Vector_of_talkers_weight(1,1:3))) ) )  ;
%     
%     
%     
%     var_vector(1,1)=var1;
%     
%     
%     
%     %Second talker vector
%     if Number_of_possible_talker>1
%         
%         % check the second talker
%         angle2=merged2_bin_value(A2_idx);
%         
%         if (A2_level/numchans>0.4)&&(Energy>Energy_threshold)
%             
%             Vector_of_talkers(2,1)= angle2;
%             Vector_of_talkers_weight(2,1)=1;
% %             display(['Second talker with probability of more than 40 percent, P>40% is at ', num2str(angle2),' degree'] )
%         else
%             Vector_of_talkers_weight(2,1)=0;
% %             display(['Second talker with probability of more than 40 percent, P<40% is at ', num2str(angle2) ,' degree, possibility of ambiguity'])
%         end
%         
%         var2=sqrt(var(Vector_of_talkers(2,1:5).*Vector_of_talkers_weight(2,1:5)));
%         var_vector(2,1)=var2;
%     end  %  check the second talker
%     check2=sum(Vector_of_talkers_weight,2);%number of appearance
%     
%     %Now time to select. criteria:1-new talker appearance(a-high var b- dropping var to zero or bellow a threshold) 2- number of
%     %appearance 3- energy
%    
%     if (Number_of_possible_talker==1)
%         if (tag_new_angle==0)% (var1>0)
%         if  (var1>0)&&(~isnan(var1))&&(~isempty(var1))%var_vector(1,4)>3
%             tag_new_angle=1;
%                   
%         end
%         end
%         if (isnan(var1))
%         tag_new_angle=0;
%         end
%         
%     
%         [num_of_var_dropping,~] =find(abs(-diff(var_vector(1,1:4)))<1);
%         
%         if var1<2;%(length(num_of_var_dropping)>2)
%             tag_new_angle2=1 ;
%         else
%             tag_new_angle2=0 ;
%         end
%         
%         if (check2>3)
%             tag_new_angle3=1   ;
%         else
%             tag_new_angle3=0   ;
%             
%         end
%         new_angle_approval=tag_new_angle*tag_new_angle2*tag_new_angle3;
%         if new_angle_approval==1
%             Output_angle=Vector_of_talkers(1,1);%the angle that send to robot
%         else %rejected? send the preceding angle
%             Output_angle=past_Output_angle;
%         end  
%         past_Output_angle=Output_angle;%update the angle
%         
%     end %talker 1 
%       Output_angle  
%         
%         end %simulation loop
        
