%sets up communication with a YARP server to send angles to the iCub

%remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');

LoadYarp;
import yarp.Port;
import yarp.Bottle;

port=Port;

port.open('/AudioAttention/AudioAngle');

pause(3);  %wait a few seconds to give everyone some time to recompose themselves