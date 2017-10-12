#!/bin/bash

build_dir="sdk-build"

if [ -d "${build_dir}" ] ; then
    rm -r ${build_dir}
fi

mkdir ${build_dir} && cp build_cmake.sh ${build_dir} && cd ${build_dir} && ./build_cmake.sh && make -j4
