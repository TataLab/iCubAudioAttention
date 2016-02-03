%  [yy,SamplingFre]=readwav('C:\Users\Home\Desktop\audioTest_right_to_left_with_distractor.wav');

function [DOA]=BeamformerFunction(GF_frameL,GF_frameR,SamplingFre)
[d1, d2]=size(GF_frameL);
if d1<d2
    numchans=d1;
else
    numchans=d2;
end

spacing=.14;%space between Mics
speedSound=340.5;% m/s
numElements=2;%Number of Mics

ANGLE_RESOLUTION=36*4; %360   % Number of angle ranges that beamformer should go through them % This means the beams moves from zero to 180 by 1 degree difference. so the spatial resolution of the beams are only 1 degree.
startAngle_index=10*(ANGLE_RESOLUTION/180); % the prepherial angles zero to 10 degrees and 170 to 180 degrees are ignored. It means that beams dont sweep these two area. This redustion in space is done to increase the speed of the algorithm
endAngle_index=170*(ANGLE_RESOLUTION/180);

% SteeringAngle is composed of angles from 10 to 170 degree by resolution of 360/ANGLE_RESOLUTION degree
SteeringAngle =  180.0 * (startAngle_index:endAngle_index) / (ANGLE_RESOLUTION-1);
SteeringAngle=SteeringAngle*pi/180;%SteeringAngle in degree
LS=length(SteeringAngle); %number of beams

%making the delay(time) and sample_delay vector for beamforming
delay = spacing * (-cos(SteeringAngle)) / speedSound;%to compensate for the delay in the left and right channel
sample_delay=round(delay*SamplingFre);
% initializing some vectors to record the results in beam sweeping itiration
logOutput=zeros(numchans,LS);

%%%Compose the Signal of beamformer here
Fframe={GF_frameL, GF_frameR}; % using the output of gammatone signal
%%% end of composing the signal

%%beamformer; sweeping beams from 10 to 170 degree for each filter channel(numchans times).

DOA=zeros(numchans,1);% initialize DOA

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
%     DOA=SteeringAngle(Index_DOA)*180/pi-90; % convert it to degree and shift by 90 degree so DOA(direction of arival) lies between -90 to 90 degree
    DOA=SteeringAngle(Index_DOA)*180/pi; % convert it to degree  DOA(direction of arival) lies between 0 to 180 degree

end% function
