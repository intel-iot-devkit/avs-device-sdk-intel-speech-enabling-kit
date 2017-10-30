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
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

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

patch_name="${base_name}_$2.bsx"
tar_name="${base_name}_$2.tar"
gzip_name="${tar_name}.gz"

cd $1
echo_info "Creating TAR file of the '$1'"
tar cf ../$tar_name ./*
check_error "Failed to create the TAR file '$tar_name'"
cd ..

echo_info "Compressing '$tar_name'"
gzip $tar_name
check_error "Failed to compress the TAR file: $tar_name"

echo_info "Creating the patch"
cat ${script_dir}/decompress.sh $gzip_name > $patch_name 
check_error "Failed to create the patch"
chmod +x $patch_name

echo_info "Cleaning up"
rm $gzip_name
check_error "Failed to clean up"

echo_info "Done."
