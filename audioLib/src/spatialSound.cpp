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

#include <iCub/audio/spatialSound.h>

using namespace yarp::sig;
using namespace yarp::os;


namespace audio
{

namespace spatialSound
{


    spatialSound::spatialSound(int bytesPerSample) {
        azimuth   = 0;
        elevation = 0;
    }
    
    /*
      spatialSound::spatialSound(const spatialSound &model) {
      valid = true;
      type = "constVelocity";
      }
    */
    
    /*
      spatialSound &spatialSound::operator =(const spatialSound &model) {
      valid = model.valid;
      type  = model.valid;
      
      return *this;
      }
    */
    
    /*
      bool spatialSound::operator ==(const spatialSound &model) {
      return ((valid==model.valid)&&(type==model.type)&&(A==model.A)&&(B==model.B)); 
      }
    */
    
    void spatialSound::init(double paramA, double paramB) {
        rowA = 3;
        colA = 3;
        
    }
    
    bool spatialSound::write(ConnectionWriter& connection) {
    }
    
    bool spatialSound::read(ConnectionReader& connection) {
        
    }

}

}
