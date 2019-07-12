# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname audio_attention_demo)

file(GLOB conf    ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini         ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.xml)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.sh )

yarp_install(FILES ${conf}    DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${scripts} DESTINATION ${ICUBCONTRIB_APPLICATIONS_TEMPLATES_INSTALL_DIR})
