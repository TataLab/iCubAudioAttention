%% For Collecting data through Beamformer
% [yy,SamplingFre]=readwav('C:\Users\Home\Desktop/audioTest_right_to_left.wav');
% [yy,SamplingFre]=readwav('C:\Users\Home\Desktop/audioTest_right_to_left_with_distractor.wav');
SamplingFre=44100;
useRobot=1;
Steps=SamplingFre*.25;
% yy=yy(16*Steps:end,:);
%%% Design GammatoneFilter between low_cf and high_cf
low_cf=500;high_cf=4000;numBands=15;
% center frequencies based on Erb scale
%YOU should  add the gammatoneFast folder to matlab path
cfs = MakeErbCFs(low_cf,high_cf,numBands);
align=true;
Threshold_Enregy=0.0001;
% Threshold_Enregy=0.00005;
Time0=(0*Steps+1:(1)*Steps)/SamplingFre;
%%

%matrices for holding previous frames of rms amplitudes
pastAmp=ones(4,numBands).*.0001; %we have to seed this with some arbitrarily small numbers
pastDeltaAmp=zeros(1,numBands);
% figure(1);clf;
% figure(3);clf;

N = 400; %Number of frames or it chould be frames*bands
% if N>floor(length(yy(:,1))/Steps)
%     N=floor(length(yy(:,1))/Steps);
% end
s=[1;30];
% true location of sound in polar coordinate system ( r, theta). For now r is always 1 and theta is variable
% theta varies from -90 to 90 and zero is at the centre.



%%
%now, we do recursive Bayesian

%define the range of locations the sound can be at
% Sa=[.6:0.2:2]; % r
Sa=1;
Sb=[0:1:180]; % theta


%no bias, no prior knowledge, uniform distribution
nn=length(Sa);
mm=length(Sb);
Pr=ones(nn,mm); % Initialize all one --> uniform prior
Po=ones(nn,mm); %duplicate for iterative update

%bias the prior toward the true location sound
%figure(1);mesh(centered_prior), axis([0 40 0 40 0 0.015])
%Po = centered_prior;
%Pr = centered_prior;

%bias the prior away from the true location sound
%figure(1);mesh(off_centered_prior), axis([0 40 0 40 0 0.015])
%Po = off_centered_prior;
%Pr = off_centered_prior;

Pri=Pr/sum(sum(Pr)); % Turn the prior into a pdf by dividing by the sum.
Poi=Po/sum(sum(Po)); % Each value is now 1/(number of states), so it all sums to one.

% figure(3);clf;plot(Po), axis([0 180 0 1])


%%iterative bayes

[a,b]=find(Po==max(max(Po)));  % Pull out the indices at which Po achieves its max to start.
sest=[Sa(a);Sb(b)];  % The best estimate of the true state to start.


Prior_count=0;
summaxPo=0;
sset_exp_num=0;


if(useRobot)
    Nstep=200;
    stepsPerDegree=Nstep/360;  %scale the step size of the motor to degrees
    
    display('Setting up Arduino and stepper motor');
    %  a = arduino('/dev/tty.usbmodem1421','uno','Libraries','Adafruit/MotorShieldV2');
    %  a = arduino('/dev/ttyACM0','uno','Libraries','Adafruit/MotorShieldV2');
    if (~exist('ar','var') || (~exist('shield','var')))
        clear ar shield
        ar = arduino('COM3','uno','Libraries','Adafruit/MotorShieldV2');
        shield=addon(ar,'Adafruit/MotorShieldV2');
        
        sm=stepper(shield,2,Nstep,'stepType','Microstep');
        %         StepType: Single ('Single', 'Double', 'Interleave', 'Microstep')
        sm.RPM = 20;
        pause(1)
    end
    
    move(sm, 1);% Just to test the arduino motor
    
else
    display('Setting up without Arduino');
end


% break
%audio capture settings
numChannels=2;
frameSize_seconds=.25;
bitDepth=16;
recorderObject=audiorecorder(SamplingFre,bitDepth,numChannels);
done=0;

SaliencyFactor_Threshold= .22;
while (~done)%for k=2:length(x); %floor(length(yy(:,1))/Steps)
    %         pause
    %     Frame=yy(k*Steps+1:(k+1)*Steps,:);
    
    
    recordblocking(recorderObject,frameSize_seconds);  %this will block until the data are acquired
    % TODO --> Use Matt function or Psycophisics toolbox
    Frame=getaudiodata(recorderObject);
    
    
    %%play sound good to have break point
    %     player = audioplayer(Frame, SamplingFre);
    %     play(player);
    %     pause(Steps/SamplingFre); % Let the system recover itself, gives it more time
    
    
    frameL=Frame(:,1);%yy(iii*Steps+1:(iii+1)*Steps,1);
    frameR=Frame(:,2);%yy(iii*Steps+1:(iii+1)*Steps,2);
    
    EnergyL_Total=trapz(Time0,abs(frameL.^2));
    EnergyR_Total=trapz(Time0,abs(frameR.^2));
    Energy=(EnergyL_Total+EnergyR_Total)/2;
    
    
    if Energy>Threshold_Enregy % to check the sailence (quiet state)
        
        
        [GF_frameL,~,~] = gammatoneFast(frameL,cfs,SamplingFre,align) ;
        [GF_frameR,~,~] = gammatoneFast(frameR,cfs,SamplingFre,align) ;
        
        %     Check Sailiency
        %         [audioSalience,  audioSalienceFactor]=AudioSaliencyFunction(frameL,frameR) ;
        run AudioSaliencyFunction.m;
        
        
        % if there is a new sound reset the Prior
        %         audioSalienceFactor
        if audioSalienceFactor>SaliencyFactor_Threshold
            Pr=Pri; % TODO decrease the prior based on time instead of resetting
            Po=Poi;
            Prior_count=0;
        end
        
        
        DOA=BeamformerFunction(GF_frameL,GF_frameR,SamplingFre); % TODO replace this function with Matt's
        %DOA is a vector of 1 by numBands, sotre angle for each Band
        
        for ff=1:numBands % number of band 
            %         ff
            
            Pr=Po; %store the posterior to the prior. Pr=P(A)
            P_AB=zeros(nn,mm);%0*Pr
            
            % look at each location, assume that the given location is
            % is there sound is, and get the likelihood of the data x(:,n)
            % 1- assuming 2-d gaussian noise 2- beamformer pattern liklihood
            
            Prior_count=Prior_count+1; % count the number of times the prior has been updated
            %             x(2,k)=DOA(ff);% by commenting this line program shift to nrandom created data set
%             x_2k=DOA(ff);
            P_BA3= pdfv2(ff, round(DOA(ff)),round((500-1)*(Sb )/180+1));%Sb(j)+1
            P_BA=reshape(P_BA3,[1,181]); %liklihood
            P_AB=P_BA .* Pr(1,:); % Combine this likelihood with the prior
            % TODO add P(B)
            Po=P_AB/sum(sum(P_AB)); %normalize this distribution to make it a proper probability distribution.
          
          
%             figure(3)
%             plot(Po), axis([1 180 0 1]) %plot it
           
            
        end
        
        [a,b]=find(Po==max(max(Po)));  % Get the peak value; it's most likely location of the sound.
        sest=[Sa(a);Sb(b)];  %store the coordinates of this location
        
        maxPo=max(max(Po));
        summaxPo=summaxPo+maxPo;
        sset_exp_num=(sset_exp_num+sest*maxPo);
        sset_exp=(sset_exp_num)/(summaxPo);
        % X=Sum(W(t)*P(Xi)*Xi)/Sum(W(t)P(Xi))
        %TODO add a weight(W(t) as a function of time( Iteration) that put
        %emphasis on the most recent ones
        
        
        
        if(useRobot)
            currentAngle=sest(2);
            steps=floor((currentAngle-90)*stepsPerDegree);
            %             if (abs(currentAngle)>5)&&(abs(currentAngle)<85)
            %
            if sset_exp>.02
                move(sm, steps);
            end
            
            
        end
        
    else
%       display('It is just a sailent or very low energy frame')
        Pr=Pri;
        Po=Poi; %TODO again decay the prior gradually
        Prior_count=0;
        summaxPo=0;
        sset_exp_num=0;
    end % Energy
    
    
    %%%%%
    %control the robot
    %%%%%%
    
      
end %while
% subplot(211); hold off;
% subplot(212); hold off;