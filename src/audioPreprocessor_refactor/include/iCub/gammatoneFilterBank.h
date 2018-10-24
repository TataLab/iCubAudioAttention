// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2018 Department of Neuroscience - University of Lethbridge
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
 * @file  gammatoneFilterBank.h
 * @brief Object for processing raw audio into a bank of frequency 
 *         bands using a 4th order gammatone filter.
 * =========================================================================== */

#ifndef _GAMMATONE_FILTER_BANK_H_
#define _GAMMATONE_FILTER_BANK_H_

#define BW_CORRECTION      1.0190
#define VERY_SMALL_NUMBER  1e-100

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>

class GammatoneFilterBank { 

  private:
    
    const double _pi = 2 * acos(0.0); //-- High precision pi.
    
	/* ===========================================================================
	 *  Variables set in the constructor. Should not be changed for life of object.
	 * =========================================================================== */
    int  numMics;
	int  samplingRate;
	int  numFrameSamples;
	int  numBands;
	int  lowCf;
	int  highCf;
	bool halfRec;
	bool erbSpaced;
	int  degreeRes;

	double tpt;
	yarp::sig::Vector cfs;


  public:

	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param mics    : Number of Microphones.
	 * @param samples : Sampling Rate the Audio was Captured at.
     * @param frames  : Number of Samples in a Frame.
     * @param bands   : Number of Frequency Bands.
     * @param lcf     : Lowest Center Frequency.
     * @param hcf     : Highest Center Frequency.
     * @param hrec    : Enable Half-Wave Rectifying.
     * @param erbs    : Enable ERB Spaced Center Frequencies.
	 * =========================================================================== */
	GammatoneFilterBank(int mics, int samples, int frames, int bands, int lcf, int hcf, bool hrec, bool erbs);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~GammatoneFilterBank();


    /* ===========================================================================
	 *  Apply a 4th order gammatone filter onto the previously set raw audio.
	 * 
     * @param filterBank : target for the filtered audio (number of mics * number of bands, number of samples).
	 * @param RawAudio   : raw audio (number of mics, number of samples).
	 * =========================================================================== */
    void getGammatoneFilteredAudio(yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& RawAudio);


  private:
	
	/* ===========================================================================
	 *  Fill the vector cfs with erb spaced center frequencies.
	 * =========================================================================== */
	void makeErbCFs();


	/* ===========================================================================
	 *  Fill the vector cfs with linear spaced center frequencies.
	 * =========================================================================== */
	void makeLinearCFs();

	
	/* ===========================================================================
	 *  Macro for converting Frequency to Equivalent Rectangular Bandwidth.
	 * =========================================================================== */
	double HzToErb(double Hz);


	/* ===========================================================================
	 *  Macro for converting Frequency to Equivalent Rectangular Bandwidth Rate.
	 * =========================================================================== */
	double HzToErbRate(double Hz);


	/* ===========================================================================
	 *  Macro for converting Equivalent Rectangular Bandwidth Rate to Frequency.
	 * =========================================================================== */
	double ErbRateToHz(double Erb);	
};

#endif //_GAMMATONE_FILTER_BANK_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
