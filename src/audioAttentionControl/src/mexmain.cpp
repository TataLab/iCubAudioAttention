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
  //help: Initialisation must include three parameter: receiving port name (string), angle (double), saliency(double).

  // initialization
  yarp::os::Network yarp;
  const mxArray *a_in_m, *b_in_m, *c_in_m;
  const mxArray *d_out_m, *e_out_m;
  double *a;
  double *b, *c;
  double *d, *e;
  const mwSize *dims;

  //figure out dimensions
  //dims = mxGetDimensions(prhs[0]);
  //numdims = mxGetNumberOfDimensions(prhs[0]);
  //dimy = (int)dims[0]; dimx = (int)dims[1];

  
  // Initialisation of the component, i.e first call after a 'close all' or matlab start
  if (nrhs < 3) {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:invalidNumInputs","Initialisation has not been performed correctly.");
  }
  

  // Check to be sure input is of type char 
  if (!(mxIsChar(prhs[0]))) {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must have receiver port name as first parameter.\n.");
  }
  if(nrhs == 3) {
    if ((mxIsChar(prhs[1]))) {
      mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must include relative angle as second parameter.\n.");
    }
    if ((mxIsChar(prhs[2]))) {
      mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must include sal as second parameter.\n.");
    }  
  }
  else {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must include three parameter: receiving port name (string), angle (double), saliency(double).\n.");
  }

  a_in_m = prhs[0];
  b_in_m = prhs[1];
  c_in_m = prhs[2];
  
  //associate pointers
  b = mxGetPr(prhs[1]);
  c = mxGetPr(prhs[2]);

  //extract string
  int buflen = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;  /* Get the length of the input string. */
  char* input_buf;
  mexPrintf("the input string contains %d chars \n", buflen);
  
  input_buf = (char*) mxCalloc(buflen, sizeof(char));      /* Allocate memory for input and output strings. */
  /* Copy the string data from prhs[0] into a C string 
   * input_buf. */
  bool status = mxGetString(prhs[0], input_buf, buflen);
  if (status != 0) {
    mexWarnMsgTxt("Not enough space. String is truncated.");
  }
  mexPrintf("Parameters %f %f %s \n",*b, *c, input_buf);

  return;
  //******************************************************************************************************************************

  BufferedPort<Bottle> bufferPort;
  if(!bufferPort.open("/audioAttentionInterface/feedback:o")) {
    mexPrintf("Error: impossible to open the port.");
    return;
  } 
  
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

  Contact contactPort = Network::queryName("/receiver");  
  mexPrintf("got into Contact \n");

  
  if(!contactPort.isValid()){
    //Port bufferPort;
    //bufferPort.open("/receiver");
    mexPrintf("receiver missing. Creating... \n");
    return;
  }
  else {
    mexPrintf("receiver valid contact \n");
    p.open(contactPort);
  }
  return;
  */
  
  if(!Network::connect("/audioAttentionInterface/feedback:o", "reader")) {
    mexPrintf("Error! Impossible to connect the port to the %s \n", input_buf );
    return;
  }
  
  Bottle bottleSent = bufferPort.prepare();
  bottleSent.clear();
  bottleSent.addDouble(*b);
  bottleSent.addDouble(*c);
  bufferPort.write();	
  	
  return;
}


