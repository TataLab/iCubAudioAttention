set(appname objectRecognitionInfer)

file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.*)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template)

yarp_install(FILES ${conf} DESTINATION .)
yarp_install(FILES ${scripts} DESTINATION .)
