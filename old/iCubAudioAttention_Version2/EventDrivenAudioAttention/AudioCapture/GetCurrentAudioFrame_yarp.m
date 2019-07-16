


function [ audioFrame ] = GetCurrentAudioFrame_yarp( audioBottle, port)
%open a port for a frame of data from Rea's RemoteInterface streaming from
%iCub pc104
%remember to add yarp to the MATLAB java path:  javaaddpath('/Users/Matthew/Documents/Robotics/yarp/bindings/java/');


%note that the frame size now is determined entirely on the pc104 side


port.read(audioBottle,true);
disp(audioBottle.toString_c());



audioFrame=b;




end

