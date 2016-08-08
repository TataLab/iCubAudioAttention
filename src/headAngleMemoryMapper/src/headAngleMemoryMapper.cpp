#include <stdio.h>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;


int main()
{
  Network yarp;


  Property options;
  options.put("device", "remote_controlboard");
  options.put("local", "/AudioFocusAttention");
  options.put("remote", "/icub/head");

  PolyDriver robotHead(options);


  if (!robotHead.isValid()) {
    printf("Cannot connect to robot head\n");
    return 1;
  }
  IPositionControl *pos;
  IVelocityControl *vel;
  IEncoders *enc;

  robotHead.view(pos);
  robotHead.view(vel);
  robotHead.view(enc);

  double* pies  = NULL; // Pointer initialized with null
  pies  = new double;   // Request memory for the variable

  FILE *fid;

  fid = fopen("/tmp/headAngleMemMap.tmp", "w");
  fwrite(pies, sizeof(double), sizeof(pies), fid);
  fclose(fid);
  int mappedFileID;
  mappedFileID = open("/tmp/headAngleMemMap.tmp", O_RDWR);
  double *mappedAudioData;
  mappedAudioData = (double *)mmap(0, sizeof(double), PROT_WRITE, MAP_SHARED , mappedFileID, 0);


  if (pos == NULL || vel == NULL || enc == NULL) {
    printf("Cannot get interface to robot head\n");
    robotHead.close();
    return 1;
  }

  int jnts = 0;
  pos->getAxes(&jnts);
  Vector setpoints;
  setpoints.resize(jnts);
  Vector checkpoints;
  checkpoints.resize(jnts);

  double oldLocation = 0;
  while (1) {
    usleep(40000);

    enc->getEncoders (checkpoints.data());
    if (oldLocation != (double) checkpoints[0]) {
      *mappedAudioData = (double) checkpoints[0];
      oldLocation = (double) checkpoints[0];
       printf ( "%f \n", *mappedAudioData);
    }
   
  }


  return 0;

}