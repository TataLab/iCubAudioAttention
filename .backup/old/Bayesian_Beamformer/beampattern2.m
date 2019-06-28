function logOutput=beampattern2(N,d,f,SteeringAngle)


ANGLE_RESOLUTION=500;    % Number of angle points to calculate
SteeringAngle=SteeringAngle*pi/180;%SteeringAngle in degree

numElements = N;         % Number of array elements
spacing = d;        % Element separation in metres
freq = f;        %Signal frequency in Hz
speedSound = 343.0;  % m/s



% Iterate through arrival angle points
for a=0:ANGLE_RESOLUTION-1
    
    % Calculate the planewave arrival angle

    
    angle =  180.0*a  / (ANGLE_RESOLUTION-1);
    angleRad = pi *  angle / 180;
    
%     if abs((angleRad-SteeringAngle))>5*pi/180
% %     weight=(cos(angleRad-SteeringAngle))^4;
%     weight=0;
%     else
%     weight=1;
%     end
    
    realSum = 0;
    imagSum = 0;
    
    %Iterate through array elements
    for i=0:numElements-1
        
        % Calculate element position and wavefront delay
        position = i * spacing;
        delay = position * (cos(angleRad)-cos(SteeringAngle)) / speedSound;
        
        %Add Wave
%         if (angleRad<88*pi/180)||(angleRad>92*pi/180)
%         realSum = realSum+ (sqrt(2)/2)*cos(2.0 * pi * freq * delay);
%         imagSum = imagSum+ (sqrt(2)/2)*sin(2.0 * pi * freq * delay);
%         else
%         realSum = realSum+ cos(2.0 * pi * freq * delay);
%         imagSum = imagSum+ sin(2.0 * pi * freq * delay);  
%         end
        
        realSum = realSum+ cos(2.0 * pi * freq * delay);
        imagSum = imagSum+ sin(2.0 * pi * freq * delay);  
        
        
                
    end
    output = sqrt(realSum *realSum + imagSum *imagSum) / numElements;
    logOutput(a+1) = 20*log10(output);
        if (logOutput(a+1) < -30)
            logOutput(a+1) = -30;
            
        end
        
    
    
end

end

%%%example & plot
% angle =  180.0 * (0:500-1) / (500-1);
% angleRad = pi *  angle / 180;
% am=beampattern2(10,0.08,2000);
% plot(angle,am)