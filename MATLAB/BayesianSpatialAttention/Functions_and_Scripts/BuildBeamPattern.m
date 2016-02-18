function [ outputBeamPattern ] = BuildBeamPattern( P )
%take struct of parameters and compute the sensitivity of a steared beam to
%off-beam arrival angles

%this will be in mic space because we need to use it in Bayes equation
%which means we'll divide by the energy in pre-recorded noise...which we'll only have at beam angles

display('Building synthetic beam pattern');

%for accumulating the rms amplitude at each frequency x arrival angle x
%steering angle
outputBeamPattern=zeros(P.nBands,P.numSpaceAngles, P.numSpaceAngles);


%for each frequency band
for bandIndex=1:P.nBands
    
    frequencyBand=P.cfs(bandIndex); %look up the center frequency of this band
    
    
    %for accumulating the beam pattern for each band independently
    thisBeamPattern=zeros(1,P.nBeams);
    
    
    %for each true arrival angle A in radians
    for arrivalAngleIndex=1:P.numSpaceAngles
        
        %we need the probability that each steering angle B will be the
        %evidence angle
        for steeringAngleIndex=1:P.numSpaceAngles
            
            arrivalAngle_radians=P.spaceAngles(arrivalAngleIndex); %look up the angle of this beam
            steeringAngle_radians=P.spaceAngles(steeringAngleIndex); %look up the steering angle of this beam
            
            %for accumulating the summed signals from each sensor
            realSum=0;
            imaginarySum=0;
            
            
            %for each microphone in the array
            for micNum=0:P.nMics-1  %start at zero because first mic is "zero" distance thus zero delay
                
                micPosition= micNum * P.D;
                
                lag = micPosition * (sin(arrivalAngle_radians) - sin(steeringAngle_radians)) / P.c;
                
                %add the signals from each microphone together
                realSum = realSum + cos(2*pi*frequencyBand*lag);
                imaginarySum = imaginarySum + sin(2*pi*frequencyBand*lag);
                
            end
            
            %compute something like RMS for each signal
            thisBeamPattern(steeringAngleIndex) = sqrt(realSum^2 + imaginarySum^2)/P.nMics;
            
            
            
            
        end
        
        
        %express into decibels so it's a sensitivity
        thisBeamPattern=20 * log10(thisBeamPattern);
        
        %pin the maximimum nulling at -50 dB so it doesn't go to -infinity
        thisBeamPattern(thisBeamPattern<-100)=-50;
        
        %shift it up so every value is positive
        thisBeamPattern=thisBeamPattern+abs(min(thisBeamPattern));
        
        
        %normalize the beam pattern so that it can work like a probability
        thisBeamPattern=thisBeamPattern./sum(thisBeamPattern); %elements sum to 1.0
        
        %accumulate this beamPattern into the matrix of output beam patterns
        outputBeamPattern(bandIndex,arrivalAngleIndex,:)=thisBeamPattern;
        
        
    end
    

    
end

display('done');
end

