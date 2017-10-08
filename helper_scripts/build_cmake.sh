#!/bin/bash

##
## CMAKE command used to compile the AVS Device SDK
##

### KITT API flagsj
### -DKITTAI_KEY_WORD_DETECTOR=ON \
### -DKITTAI_KEY_WORD_DETECTOR_LIB_PATH=${PWD}/../snowboy/lib/ubuntu64/libsnowboy-detect.a \
### -DKITTAI_KEY_WORD_DETECTOR_INCLUDE_DIR=${PWD}/../snowboy/include \
    # -DSOCKET_HARDWARE_CONTROLLER=ON \

cmake -DCMAKE_BUILD_TYPE=DEBUG \
    -DHARDWARE_KEY_WORD_DETECTOR=ON \
    -DALSA_HARDWARE_CONTROLLER=ON \
    -DGSTREAMER_MEDIA_PLAYER=ON \
    -DPORTAUDIO=ON\
    -DPORTAUDIO_LIB_PATH=${PWD}/../portaudio/lib/.libs/libportaudio.a \
    -DPORTAUDIO_INCLUDE_DIR=${PWD}/../portaudio/include/ \
    ../avs_integration-sue_integration/

