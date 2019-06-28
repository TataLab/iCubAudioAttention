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


    #define HELPER(x) (*((FlexImage*)(x)))

    spatialSound::spatialSound(int bytesPerSample){
        azimuth   = 0;
        elevation = 0;
        init(bytesPerSample);
        frequency = 0;
        numberOfAngles = 2; // hardcoded number of angles azimuth and elevation
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
        azimuth   = -1.0;
        elevation = -1.0;
    }

    /*
    void spatialSound::init(int bytesPerSample) {
        bytesPerSample = 4;
        implementation = new FlexImage();
        yAssert(implementation!=NULL);

        //yAssert(bytesPerSample==2); // that's all thats implemented right now

        if(bytesPerSample == 2) {
            HELPER(implementation).setPixelSize(sizeof(PixelMono16));
            HELPER(implementation).setPixelCode(VOCAB_PIXEL_MONO16);
            HELPER(implementation).setQuantum(2);
        }
        else if (bytesPerSample==4){
            HELPER(implementation).setPixelSize(sizeof(PixelInt));
            HELPER(implementation).setPixelCode(VOCAB_PIXEL_INT);
            HELPER(implementation).setQuantum(4);
        }

        //samples = 0;
        //channels = 0;
        //this->bytesPerSample = bytesPerSample;
        }*/

    void spatialSound::setAngles(Vector v) {
        azimuth = v[0];
        elevation = v[1];
        //vergence = v[2];
    }

    
    /*void spatialSound::synchronize() {
        FlexImage& img = HELPER(implementation);
        //samples = img.width();
        //channels = img.height();
        }*/

    void spatialSound::resize(int samples, int channels) {
        FlexImage& img = HELPER(implementation);
        img.resize(samples, channels + numberOfAngles);
        synchronize();
    }
    
    bool spatialSound::write(ConnectionWriter& connection) {
        // lousy format - fix soon!
        FlexImage& img = HELPER(implementation);
        Bottle bot;
        bot.addInt(frequency);
        bot.addDouble(azimuth);
        bot.addDouble(elevation);
        return PortablePair<FlexImage,Bottle>::writePair(connection,img,bot);  
    }
    
    bool spatialSound::read(ConnectionReader& connection) {
        // lousy format - fix soon!
        FlexImage& img = HELPER(implementation);
        ;
        Bottle bot;
        bool ok = PortablePair<FlexImage,Bottle>::readPair(connection,img,bot);
        frequency = bot.get(0).asInt();
        azimuth   = bot.get(1).asDouble();
        elevation = bot.get(2).asDouble();
        int bytesPerSample = bot.get(1).asInt();

        if(bytesPerSample == 2) {
            img.setPixelSize(sizeof(PixelMono16));
            img.setPixelCode(VOCAB_PIXEL_MONO16);
            img.setQuantum(2);
        }
        else if (bytesPerSample==4){
            img.setPixelSize(sizeof(PixelInt));
            img.setPixelCode(VOCAB_PIXEL_INT);
            img.setQuantum(4);
        }

        yDebug("read %d img pixelSize %d", bytesPerSample, HELPER(implementation).getPixelSize());
        synchronize();
        return ok;
    }



}
