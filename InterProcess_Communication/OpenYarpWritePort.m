function [ port ] = OpenYarpWritePort(  )
%Load yarp, open a port
%remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');

%setup


LoadYarp;
import yarp.Port;
import yarp.Bottle;

port=Port;
%first close the port just in case
port.close;

disp('Going to open port /audioAttention/audioAngle');
port.open('/audioAttention/audioAngle');

disp('Please connect to a bottle sink (e.g. yarp read)');

return;
end

