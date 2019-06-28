// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/* 
 * Copyright (C) 2017 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Francesco Rea
 * email:  francesco.rea@iit.it
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

#ifndef __SPATIAL_SOUND_H__
#define __SPATIAL_SOUND_H__

#include <yarp/os/all.h>
#include <yarp/sig/Sound.h>
#include <yarp/os/PortablePair.h>
#include <yarp/os/ConnectionWriter.h>
#include <yarp/os/ConnectionReader.h>
#include <string>
#include <deque>

namespace audio {

	namespace spatialSound {

		class spatialSound:yarp::sig::Sound {

		  protected:

			bool valid;                 // defines the validity of the model
			std::string type;           // defines the typology of the model
   
			int inputId;                // reference to the input value
			int rowA,colA;              // dimension of the matrix A
			double paramA;              // paramA 
			double paramB;              // paramB
			double azimuth, elevation;  //


		  public:

			spatialSound(int bytesPerSample);             //constructor
			spatialSound(const spatialSound &model);
			bool isValid() const        { return valid; }
			//std::string getType() const { return type;  }
		
			int               getRowA()   const    {return rowA;   };
			int               getColA()   const    {return colA;   };
			double            getParamA() const    {return paramA; };
			double            getParamB() const    {return paramB; };

			/**
			 * @brief function for the initialisation of the kalman filter
			 * @param param1 first parameter ( only one in 1-Dimension space) 
			 * @param param2 second paramter (eventually NULL in 1-Dimension space)
			 */
			void init(double param1, double param2 = 0);
			bool write(yarp::os::ConnectionWriter& connection);
			bool read(yarp::os::ConnectionReader& connection);
			//virtual bool operator ==  (const predModel &pModel);

		};
	} // spatialSound
} // audio

#endif


