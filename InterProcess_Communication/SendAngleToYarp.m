function SendAngleToYarp( angle,port )
%take an angle, put it in a bottle, send it away

%LoadYarp;

import yarp.Bottle;

angleBottle=Bottle;

angleBottle.addDouble(angle);
port.write(angleBottle);
end

