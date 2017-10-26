#!/bin/bash

##
## Simple bash script to help with creating patches to the sdk-build
##

## Logging
RED='\033[0;31m'
YELLOW="\033[1;33m"
GREEN="\033[0;32m"
NC='\033[0m' # No Color
cwd=`pwd`

function echo_info() {
    echo -e "${GREEN}`date` : INFO : $1 ${NC}"
}

function echo_warn() {
    echo -e "${YELLOW}`date` : WARN : $1 ${NC}"
}

function echo_error() {
    echo -e "${RED}`date` : ERROR : $1 ${NC}"
}

function check_error() {
    if [ $? -ne 0 ] ; then
        echo_error "ERROR: $1"
        exit -1
    fi
}

base_name="rp_cpp_patch"

if [ "$#" -ne 2 ] ; then
    echo_error "Incorrect number of arguments"
    echo "usage: $0 <build-dir> <patch-id>"
    echo -e "\tbuild-dir - C++ SDK build directory to use"
    echo -e "\tpatch-id  - ID of the patch"
    exit -1
fi

if [ ! -d "$1" ] ; then
    echo_error "C++ SDK build directory '$1' does not exist"
    exit -1
fi

dest="${base_name}_$2.zip"

if [ -f "$dest" ] ; then
    echo_error "Patch with name '$dest' already exists"
    exit -1
fi

echo_info "Creating patch $dest"
zip -r $dest $1
check_error "Failed to create patch $dest"

echo_info "Done."
