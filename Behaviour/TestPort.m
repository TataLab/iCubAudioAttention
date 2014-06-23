LoadYarp;
import yarp.Port;
import yarp.Bottle;

port=Port;

port.open('/test/test');

pause(10);
display('sending a 10 to yarp');
import yarp.Bottle;

angleBottle=Bottle;

angleBottle.addDouble(10.0);
port.write(angleBottle);