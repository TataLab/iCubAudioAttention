# Copyright: (C) 2013 Istituto Italiano di Tecnologia
# Author: Elena Ceseracciu
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

if (NOT audioLib_FOUND)

set(audioLib_LIBRARIES "audioLib" CACHE INTERNAL "List of audioLib libraries")

include("/usr/local/lib/audioLib/audioLib-export-install.cmake")
include("/usr/local/lib/audioLib/audioLib-export-install-includes.cmake")

set (audioLib_FOUND TRUE)
endif (NOT audioLib_FOUND)
