#!/bin/bash

##
## Simple bash script to help with creating images from a given SD card. This
## script simply uses dd to create an image of the SD card given by the first
## command line parameter, and then compresses that image into a zip file with
## the name rp_cpp_<id>.img.zip. The <id> is given by the second command line
## parameter.
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

function verify_root() {
    if [[ $EUID -ne 0 ]]; then
        echo_error "Script must be ran as root"
        exit -1
    fi
}

# Verify that we are running as root
verify_root

if [ "$#" -ne 2 ] ; then
    echo_error "Incorrect number of arguments"
    echo "usage: $0 <device> <bkc-id>"
    echo -e "\tdevice - SD Card to DD into an image"
    echo -e "\tbkc-id - ID of the BKC"
    exit -1
fi

if [ ! -b "$1" ] ; then
    echo_error "Device \"$1\" does not exist"
    exit -1
fi

base_fn="rp_cpp_$2.img"
zip_fn="${base_fn}.zip"

if [ -f "$base_fn" ] ; then
    echo_error "BKC already exists"
    exit -1
fi

if [ -f "$zip_fn" ] ; then
    echo_error "Packaged BKC already exists"
    exit -1
fi

echo_info "Creating image $base_fn"
dd if=$1 | pv | dd bs=4M of=$base_fn
check_error "Failed to creating image $base_fn"

echo_info "Compressing $base_fn"
zip $zip_fn $base_fn
check_error "Failed to compress $base_fn into $zip_fn"

echo_info "DONE."
