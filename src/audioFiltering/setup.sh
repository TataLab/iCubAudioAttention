#!/bin/sh

if [ ! -d $1/webrtc-audio-processing ];then
  git clone https://github.com/TataLab/webrtc-audio-processing $1/webrtc-audio-processing
fi

if [ ! -d $1/webrtc-audio-processing/release ];then
  cd $1/webrtc-audio-processing
  mv RELEASE release.txt >/dev/null
  ./autogen.sh --prefix=`pwd`/release && make -j && make install && cd -
fi
