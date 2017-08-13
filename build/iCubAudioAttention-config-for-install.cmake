# Copyright: (C) 2013 Istituto Italiano di Tecnologia
# Author: Elena Ceseracciu
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

if (NOT iCubAudioAttention_FOUND)

set(iCubAudioAttention_LIBRARIES "audioLib" CACHE INTERNAL "List of iCubAudioAttention libraries")

include("/usr/local/lib/iCubAudioAttention/iCubAudioAttention-export-install.cmake")
include("/usr/local/lib/iCubAudioAttention/iCubAudioAttention-export-install-includes.cmake")

set (iCubAudioAttention_FOUND TRUE)
endif (NOT iCubAudioAttention_FOUND)
