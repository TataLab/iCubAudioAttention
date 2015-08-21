/*
 * Copyright (C) 2014 Robotics, Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Authors: Francesco Rea 
 * email: francesco.rea@iit.it
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */


// global includes
#include <iostream> 
#include <stdio.h>
#include <mex.h>

// include yarp 
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Sound.h>
#include <yarp/os/NetInt16.h>

// Local includes
//#include <modelcomponent.h>
//#include <componentmanager.h>

// Namespaces
using namespace yarp::os;
using namespace yarp::sig;
//using namespace mexWBIComponent;

//Global variables
//static ComponentManager *componentManager = NULL;
const NetInt16 normDivid= 32768;   //normalised signed 16bit int into signed [-1.0,+1.0] 


//=========================================================================================================================
// Entry point function to library
void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
  //mexPrintf("happily starting the procedure \n");

  // initialization
  yarp::os::Network yarp;
  //const mxArray *a_in_m, *b_in_m;
  const mxArray *c_out_m, *d_out_m, *e_out_m;
  double *a, *b;
  double *c, *d, *e;
  const mwSize *dims;

  BufferedPort<bottle> headAnglePort;
  bufferPort.open("/audioAttentionFeedback/headAngle:i");
 
  /*
  if(!Network::exists("/receiver")){
    mexPrintf("receiver not exists \n");
    bufferPort.open("/receiver");
  }
  else{
    mexPrintf("receiver exists \n");
  }

  if(!Network::exists("/receiver")) {
    mexPrintf("error \n");
  }

  if(!Network::isConnected("/sender", "/receiver")) {
    mexPrintf("no connection \n");
    Network::connect("/sender", "receiver");
  }
  else {
    mexPrintf("connected alrteady \n");
  }  
  */
  
  //hard coded YARP stuff.  Might need to be changed in the future.  It needs to match the PC104 side.
  if (!Network::connect("/icub/head/state:o", "/audioAttentionFeedback/headAngle:i")) {
    mexPrintf("error \n");
    *c = 0;
    return;
  }
  
  //associate outputs
  c_out_m = plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
  //associate pointers
  c = mxGetPr(plhs[0]);
  
  Bottle *receivedBottle = bufferPort.read(true);
  *c = receivedBottle.get(0).asDouble();  	  
 
  return;
}


