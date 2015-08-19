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

  //figure out dimensions
  //dims = mxGetDimensions(prhs[0]);
  //numdims = mxGetNumberOfDimensions(prhs[0]);
  //dimy = (int)dims[0]; dimx = (int)dims[1];

  /***** removed section Rea August 14th 2015 **************************
  // Initialisation of the component, i.e first call after a 'close all' or matlab start
  if (nrhs < 1) {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:invalidNumInputs","Initialisation has not been performed correctly.");
  }
  

  // Check to be sure input is of type char 
  if (!(mxIsChar(prhs[0]))) {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must include component.\n.");
  }
  if(nrhs ==2) {
    if (!(mxIsChar(prhs[1]))) {
      mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must include component and a robot name.\n.");
    }
    mexPrintf("Correct number of parameters");  
  }
  else {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Initialisation must include component and a robot name.\n.");
  }
   
  //mexPrintf("starting to process function\n");
  
  if (nrhs < 1) {
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:invalidNumInputs","Required Component must be named.");
  }  
  
  // Check to be sure input is of type char 
  if (!(mxIsChar(prhs[0]))){
    mexErrMsgIdAndTxt( "MATLAB:mexatexit:inputNotString","Input must be of type string.\n.");
  }
  ***********************************************************************/

  BufferedPort<Sound> bufferPort;
  bufferPort.open("/receiver");
  
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
  
  Network::connect("/sender", "/receiver");
  //yarp.connect("/sender","/receiver");
  
 
  
  Sound* s;
  int nchannels = 2;           // number of channels
  int rate = 48000;            // sampling rate
  int nbytes = 2;              // number of bytes
  double time = 0.1;                // sec
  int nsamples = 4096;  // number of samples
  
  //associate outputs
  c_out_m = plhs[0] = mxCreateDoubleMatrix(1,nsamples, mxREAL);
  d_out_m = plhs[1] = mxCreateDoubleMatrix(1,nsamples, mxREAL);
  e_out_m = plhs[2] = mxCreateDoubleMatrix(1,2, mxREAL);
  
  //c_out_m = plhs[0] = mxCreateNumericMatrix(1,rate * time, mxINT16_CLASS, mxREAL);
  //d_out_m = mxCreateNumericMatrix(1,rate * time, mxINT16_CLASS, mxREAL);

  //associate pointers
  c = mxGetPr(plhs[0]);
  d = mxGetPr(plhs[1]);
  e = mxGetPr(plhs[2]);
  
  Stamp ts;	
  
  	s = bufferPort.read(true);
  	bufferPort.getEnvelope(ts);
  	//mexPrintf("count:%d time:%f \n", ts.getCount(), ts.getTime());
  	e[0] = ts.getCount();
  	e[1] = ts.getTime();
  	
  	//int nsamples  = rate * time * nchannels * nbytes;
  
  	//if(s!=NULL) {
  	//  unsigned char* soundData = s->getRawData();
  	//}
  	//else{
  	//  mexPrintf("Cycle skipped because of the null value of the sound pointer \n");
  	//}
  
  	//unsigned char* dataSound;

  	//if(s!=NULL) {
  	//  dataSound = s->getRawData(); 
  	//}
  	//else {
  	//  mexPrintf("NULL DATA");
  	//}

  	for (int j = 0; j < nsamples; j++) {
    	NetInt16 temp_c = (NetInt16) s->get(j,0);
    	c[j] = (double) temp_c / normDivid ;
    	NetInt16 temp_d = (NetInt16) s->get(j,1);
    	d[j] = (double) temp_d / normDivid;
  	}

  	//bufferPort.interrupt();
  	//bufferPort.close();

  	//mexPrintf("end. \n");

  	/*while (true) {
    		s = bufferPort.read(true);
    		if (s!=NULL){
    			mexPrintf("received \n");
    			//put->renderSound(*s);
    		}
    	}
  	*/
  
  return;
}


