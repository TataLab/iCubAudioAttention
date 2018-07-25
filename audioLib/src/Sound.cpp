/*
 * Copyright (C) 2006 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */


#include <iCub/audio/Sound.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/PortablePair.h>
#include <yarp/os/Log.h>
#include <yarp/os/Value.h>
#include <yarp/os/NetUint32.h>

#include <cstring>
#include <cstdio>

//using namespace yarp::sig;
using namespace yarp::os;

using namespace audio;

#define HELPER(x) (*((yarp::sig::FlexImage*)(x)))

Sound::Sound(int bytesPerSample) {
    init(bytesPerSample);
    frequency = 0;
}

Sound::Sound(const Sound& alt) : yarp::os::Portable() {
    init(alt.getBytesPerSample());
    yarp::sig::FlexImage& img1 = HELPER(implementation);
    yarp::sig::FlexImage& img2 = HELPER(alt.implementation);
    img1.copy(img2);
    frequency = alt.frequency;
    synchronize();
}

Sound& Sound::operator += (const Sound& alt) {
    if (alt.channels!= channels)
    {
        printf ("unable to concatenate sounds with different number of channels!");
        return *this;
    }
    if (alt.frequency!= frequency)
    {
        printf ("unable to concatenate sounds with different sample rate!");
        return *this;
    }

    int offset = this->getRawDataSize();
    int size   = alt.getRawDataSize();

    Sound orig= *this;
    this->resize(this->samples+alt.samples,channels);

    unsigned char* p1    = orig.getRawData();
    unsigned char* p2    = alt.getRawData();
    unsigned char* pout  = this->getRawData();

    for (int i=0; i<offset; i++)
        pout[i]=p1[i];

    int j=0;
    for (int i=offset; i<offset+size; i++)
        pout[i]=p2[j++];

    this->synchronize();
    return *this;
}

const Sound& Sound::operator = (const Sound& alt) {
    yAssert(getBytesPerSample()==alt.getBytesPerSample());
    yarp::sig::FlexImage& img1 = HELPER(implementation);
    yarp::sig::FlexImage& img2 = HELPER(alt.implementation);
    img1.copy(img2);
    frequency = alt.frequency;
    synchronize();
    return *this;
}

void Sound::synchronize() {
    yarp::sig::FlexImage& img = HELPER(implementation);
    samples = img.width();
    channels = img.height();
}

Sound Sound::subSound(int first_sample, int last_sample)
{
    if (last_sample  > this->samples)
        last_sample = samples;
    if (first_sample > this->samples)
        first_sample = samples;
    if (last_sample  < 0)
        last_sample = 0;
    if (first_sample < 0)
        first_sample = 0;
    if (last_sample < first_sample)
        last_sample = first_sample;

    Sound s;

    s.resize(last_sample-first_sample, this->channels);
    s.setFrequency(this->frequency);

    /*
    //faster implementation but currently not working
    unsigned char* p1    = this->getRawData();
    unsigned char* p2    = s.getRawData();
    int j=0;
    for (int i=first_sample; i<last_sample*2; i++)
    {
        p2[j++]=p1[i];
    }
    */

    //safe implementation
    int j=0;
    for (int i=first_sample; i<last_sample; i++)
    {
        for (int c=0; c< this->channels; c++)
            s.set(this->get(i,c),j,c);
        j++;
    }

    s.synchronize();

    return s;
}

void Sound::init(int bytesPerSample) {
  
    implementation = new yarp::sig::FlexImage();
    yAssert(implementation!=NULL);
    bytesPerSample=4;
    //yAssert(bytesPerSample==2); // that's all thats implemented right now
    if(bytesPerSample == 2) {
      HELPER(implementation).setPixelSize(sizeof(yarp::sig::PixelMono16));
      HELPER(implementation).setPixelCode(VOCAB_PIXEL_MONO16);
      HELPER(implementation).setQuantum(2);
    }
    else if (bytesPerSample==4){
      HELPER(implementation).setPixelSize(sizeof(yarp::sig::PixelInt));
      HELPER(implementation).setPixelCode(VOCAB_PIXEL_INT);
      HELPER(implementation).setQuantum(4);
    }
     
    samples = 0;
    channels = 0;
    //yDebug("bytesPerSample %d", bytesPerSample);
    this->bytesPerSample = bytesPerSample;
}

Sound::~Sound() {
    if (implementation!=NULL) {
        delete &HELPER(implementation);
        implementation = NULL;
    }
}

void Sound::resize(int samples, int channels) {
    yarp::sig::FlexImage& img = HELPER(implementation);
    img.resize(samples,channels);
    synchronize();
}

int Sound::get(int location, int channel) const {
    yarp::sig::FlexImage& img = HELPER(implementation);
    unsigned char *addr = img.getPixelAddress(location,channel);
    if (bytesPerSample==2) {
        return *((NetUint16 *)addr);
    }
    else if (bytesPerSample==4) {
    	//yInfo("getting sound at 4 bytes per sample");
      return *((NetUint32 *)addr);
    }
    
    //yInfo("sound only implemented for 16 bit samples");
    return 0;
}

void Sound::clear()
{
    int size = this->getRawDataSize();
    unsigned char* p  = this->getRawData();
    memset(p,0,size);
}

void Sound::set(int value, int location, int channel) {
    yarp::sig::FlexImage& img = HELPER(implementation);
    unsigned char *addr = img.getPixelAddress(location,channel);
    if (bytesPerSample==2) {
      //yInfo("sound only implemented for 16 bit samples");
      *((NetUint16 *)addr) = value;
        return;
    }
    else if (bytesPerSample==4) {
      //yInfo("sound only implemented for 32 bit samples");
      *((NetInt32 *)addr) = value;
      //printf("%d %d", value, *((NetUint32 *)addr));
      return;
    }
    
}

int Sound::getFrequency() const {
    return frequency;
}

void Sound::setFrequency(int freq) {
    this->frequency = freq;
}

bool Sound::read(ConnectionReader& connection) {
    // lousy format - fix soon!
    //yInfo("getting sound at 4 bytes per sample");
    yarp::sig::FlexImage& img = HELPER(implementation);
    //yDebug("write img pixelsize %d", img.getPixelSize());
    Bottle bot;
    bool ok = PortablePair<yarp::sig::FlexImage,Bottle>::readPair(connection,img,bot);
    frequency = bot.get(0).asInt();
    synchronize();
    return ok;
}


bool Sound::write(ConnectionWriter& connection) {
    // lousy format - fix soon!
    yarp::sig::FlexImage& img = HELPER(implementation);
    yDebug("write img pixelsize %d", img.getPixelSize());
    Bottle bot;
    bot.addInt(frequency);
    //bot.addInt(img.getPixelSize());
    return PortablePair<yarp::sig::FlexImage,Bottle>::writePair(connection,img,bot);
}

unsigned char *Sound::getRawData() const {
    yarp::sig::FlexImage& img = HELPER(implementation);
    return img.getRawImage();
}

int Sound::getRawDataSize() const {
    yarp::sig::FlexImage& img = HELPER(implementation);
    return img.getRawImageSize();
}
