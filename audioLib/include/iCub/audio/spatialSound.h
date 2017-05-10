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

#include <iCub/audio/Sound.h>

#include <yarp/os/all.h>
//#include <yarp/sig/Sound.h>
#include <yarp/sig/Image.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/PortablePair.h>
#include <yarp/os/ConnectionWriter.h>
#include <yarp/os/ConnectionReader.h>
#include <string>
#include <deque>

namespace audio
{


    class spatialSound : public audio::Sound {
protected:
    bool valid;                 // defines the validity of the model
    std::string type;           // defines the typology of the model
        //int frequency;              // frequency of the sound acquition
    int inputId;                // reference to the input value
    int rowA,colA;              // dimension of the matrix A
        int numberOfAngles;         // number (e.g: 2 angles azimuth elevation)    
        double paramA;              // paramA 
    double paramB;              // paramB
    double azimuth, elevation;  // azimuth and elevation at the moment of the acquisition.

        //void* implementation; // pointer to the implementation    
public:
        spatialSound(int bytesPerSample);             //constructor
        spatialSound(const spatialSound &model);
        bool isValid() const        { return valid; }
        std::string getType() const { return type;  }

        void setNumberOfAngles(int n) {numberOfAngles = n;};
        
        /**
         *@brief function that sets angles in the memory 
         */
        void setAngles(yarp::sig::Vector angles);
        
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
        /** 
         * @brief function for the initialisation of the spatialSound class
         * @param bytesPerSample number of bytes per sample
         */
        //void init(int bytesPerSample);

        /**
         * @brief function that resizes the image specifically for spatialSound
         * @param samples the number of samples that defines the width
         * @param channels the number os channels that defines the height (+2:azimuth, elevation)
         */
        void resize(int samples, int channels);

        /** 
         * @brief function that overwrites the write
         */
        virtual bool write(yarp::os::ConnectionWriter& connection);

        /**
         * @brief function that overwrites the read
         */
        bool read(yarp::os::ConnectionReader& connection);

        //void synchronize();
        //void resize(int samples, int channels = 1);
        //void set(int value, int sample, int channel = 0);        
        //virtual bool operator ==  (const predModel &pModel);

};





}

#endif


