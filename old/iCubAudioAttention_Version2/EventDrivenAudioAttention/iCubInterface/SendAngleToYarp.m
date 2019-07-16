function SendAngleToYarp( angle,salience,port )
%take an angle, put it in a bottle, send it away
%you can provide a salience for this angle and it will be included in the
%bottle,  if salience is NaN then only the angle is sent

%LoadYarp;

import yarp.Bottle;

angleBottle=Bottle;


angleBottle.addDouble(angle);

if (~isnan(salience))
    angleBottle.addDouble(salience);
end

port.write(angleBottle);
end

