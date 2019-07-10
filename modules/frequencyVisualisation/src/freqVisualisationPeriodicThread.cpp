// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
 * Author: Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
 * email: kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
 * 
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

/* ===========================================================================
 * @file  freqVisualisationPeriodicThread.cpp
 * @brief Implementation of the freqVisualisationPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/freqVisualisationPeriodicThread.h>

#define THPERIOD 0.01 // seconds.
// #define gain   500


/* ===========================================================================
 *  Return a RGB colour value given a scalar v in the range [vmin,vmax]
 *  In this case each colour component ranges from 0 (no contribution) to
 *  1 (fully saturated), modifications for other ranges is trivial.
 *  The colour is clipped at the end of the scales if v is outside
 *  the range [vmin,vmax]
 * =========================================================================== */

typedef struct { double r,g,b; } COLOUR;
COLOUR GetColour(double v,double vmin,double vmax) {

   COLOUR c = {1.0,1.0,1.0}; // white
   double dv;

   if (v < vmin) v = vmin;
   if (v > vmax) v = vmax;

   dv = vmax - vmin;

   if (v < (vmin + 0.25 * dv)) {
      c.r = 0;
      c.g = 4 * (v - vmin) / dv;
   } else if (v < (vmin + 0.5 * dv)) {
      c.r = 0;
      c.b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
   } else if (v < (vmin + 0.75 * dv)) {
      c.r = 4 * (v - vmin - 0.5 * dv) / dv;
      c.b = 0;
   } else {
      c.g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
      c.b = 0;
   }

   return(c);
}


FreqVisualisationPeriodicThread::FreqVisualisationPeriodicThread() : 
    PeriodicThread(THPERIOD) {

    robot = "icub";
}


FreqVisualisationPeriodicThread::FreqVisualisationPeriodicThread(std::string _robot, std::string _configFile) : 
    PeriodicThread(THPERIOD) {

    robot = _robot;
    configFile = _configFile;
}


FreqVisualisationPeriodicThread::~FreqVisualisationPeriodicThread() {
    // do nothing
}


bool FreqVisualisationPeriodicThread::configure(yarp::os::ResourceFinder &rf) {

    /* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );

    int         inGain = rf.check("gain", yarp::os::Value(1),      "Gain for visualization (int)"  ).asInt();
    std::string inGrid = rf.check("grid", yarp::os::Value("none"), "Visualisation of grid (string)").asString();
    
    this->setGain(inGain);
    this->setGrid(inGrid);


    /* =========================================================================== 
	 *  Print the resulting variables to the console.
	 * =========================================================================== */
	yInfo( " " );
	yInfo( "\t          [VISUALISATION PARAMETERS]          "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Gain Value                       : %d",       inGain         );
	yInfo( "\t Grid Style                       : %s",       inGrid.c_str() );
	yInfo( " " );

    return true;
}


bool FreqVisualisationPeriodicThread::threadInit() {
    
    /* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

    if (!inputPort.open(getName("/map:i").c_str())) {
        yError("Unable to open port to receive input.");
        return false;
    }

    if (!outputPort.open(getName("/img:o").c_str())) {
        yError("Unable to open port to send unmasked events.");
        return false;  
    }

    yInfo("Initialization of the processing thread correctly ended.");
	
    return true;
}


void FreqVisualisationPeriodicThread::threadRelease() {

    //-- Stop all threads.
    inputPort.interrupt();
    outputPort.interrupt();
    	
	//-- Close the threads.
    inputPort.close();
    outputPort.close();
}


void FreqVisualisationPeriodicThread::setName(std::string str) {
    this->name=str;
}


std::string FreqVisualisationPeriodicThread::getName(const char* p) {
    std::string str(name);
    str.append(p);
    return str;
}


void FreqVisualisationPeriodicThread::setInputPortName(std::string InpPort) {
    //-- Do nothig.
}


void FreqVisualisationPeriodicThread::run() {

    if (inputPort.getInputCount()) {

        yarp::sig::Matrix* mat = inputPort.read(false);   //blocking reading for synchr with the input
        
        if (mat!=NULL) {
            //yDebug("matrix is not null");
            if (outputPort.getOutputCount()) {
                //yDebug("preparing the image %d %d",mat->cols(),mat->rows() );
                outputImage = &outputPort.prepare();
                outputImage->resize(mat->cols(),mat->rows());
                result = processing(mat);
            }
        }
    }

    if (outputPort.getOutputCount()) {
        //yDebug("writing the output image out");

        // changing the pointer of the prepared area for the outputPort.write()
        //outputPort.prepare() = *inputImage;

        outputPort.write();
    }

}


bool FreqVisualisationPeriodicThread::addLegend() {

    int iplwidth  = outputImage->width();
    int iplheight = outputImage->height();

    IplImage* iplimage = cvCreateImage(cvSize(iplwidth,iplheight),IPL_DEPTH_8U,3);
    IplImage* iplcopy;

    outputImage->wrapIplImage(iplcopy); //-- TODO: Fix Depricated!!
    
    CvPoint centroid;
    centroid.x = 100;
    centroid.y = 1;

    CvFont font;
    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, CV_AA);
    cvPutText(iplcopy, "1m", centroid, &font, cvScalar(255, 255, 255) );
    
    return true;
}


bool FreqVisualisationPeriodicThread::processing(yarp::sig::Matrix* mat) {

    // here goes the processing...
    int nRows = mat->rows();
    int nCols = mat->cols();

    //yDebug("Matrix rows %d cols %d ", nRows, nCols );
    imageOutWidth  = mat->cols();
    imageOutHeight = mat->rows();
    unsigned char* pImage = outputImage->getRawImage();

    //yDebug("image width %d, height %d", outputImage->width(), outputImage->height());
    double* pMat = mat->data();
    double value;

    int padding = outputImage->getPadding();
    int nColsDiv4;

    //checking if two shifts are allowed
    if(nCols % 4 == 0) {
        nColsDiv4 = nCols >> 2;
    } else {
        nColsDiv4 = 1;
    }

    yInfo("nCols %d nColsDiv4 %d", nCols, nColsDiv4);

    for (int r = 0; r < nRows; r++) {
        for (int c = 0; c < nCols; c++) {

            // drawing of the line only if the width is 16pixels
            
            if ((c % nColsDiv4 == 0) && (c != 0) && (nColsDiv4 >= 4) && (grid == 2)) {

                value = *pMat;
                COLOUR col = GetColour(value * visGain, 0.0, 1.0);
                //*pImage = 0;
                *pImage = 0 * 255;
                pImage++;
                //*pImage = (unsigned char) floor(255 * gain * value);
                *pImage = 0 * 255;
                pImage++;
                //*pImage = 0;
                *pImage = 0 * 255;
                pImage++;
                pMat++;

            } else {

                value = *pMat;
                COLOUR col = GetColour(value * visGain, 0.0, 1.0);
                //*pImage = 0;
                *pImage = col.r * 255;
                pImage++;
                //*pImage = (unsigned char) floor(255 * gain * value);
                *pImage = col.g * 255;
                pImage++;
                //*pImage = 0;
                *pImage = col.b * 255;
                pImage++;
                pMat++;

            }
        }

        pImage += padding;
    }

    //-- adding leggend.
    //addLegend();
    
    return true;
}


bool FreqVisualisationPeriodicThread::processing(yarp::os::Bottle* b) {

    // here goes the processing...
    yDebug("length %ld ", b->size());
    int intA = b->get(0).asInt();
    int intB = b->get(1).asInt();
    
    yarp::os::Bottle* subBottle = b->get(2).asList();
    
    yDebug("a %d b %d", intA, intB);
    yDebug("bottle length %d: %s", 10, subBottle->toString().c_str());
    
    return true;
}


void FreqVisualisationPeriodicThread::setGain(int inGain) {
    this->visGain = inGain;
}


void FreqVisualisationPeriodicThread::setGrid(std::string str) {

    if (strcmp(str.c_str(), "vert") == 0) {
        grid = 2;
    } else if (strcmp(str.c_str(), "hor") == 0) {
        grid = 1;
    } else {
        grid = 0;
    }
}
