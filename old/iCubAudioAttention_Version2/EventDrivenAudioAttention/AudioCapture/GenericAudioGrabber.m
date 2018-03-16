



%open a yarp port and read data continuosly
LoadYarp;

import yarp.BufferedPortSound;
import yarp.Sound;

bufferPort = BufferedPortSound;
bufferPort.open('/matlab/bufferListener');

s = Sound;

[x,y]=audioFrameGrabber(bufferPort,'b');


% 
% 
% done=0;
% for i=1:3    
%     display('running');
%     s = bufferPort.read(true);
%     
% %     s=port.read(b,true);
% %    display(b.toString_c());    
%  %   pause(2);
%     
% end
% 
