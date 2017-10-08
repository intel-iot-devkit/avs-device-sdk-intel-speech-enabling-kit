#!/bin/bash

if [ -d "avs_build" ] ; then
    rm -r avs_build
fi

mkdir avs_build && cp build_cmake.sh avs_build && cd avs_build && ./build_cmake.sh && make -j4
