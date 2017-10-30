#!/bin/bash

##
## Decompression Script
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

sdk_folder="$HOME/sdk-folder"
sdk_build="$sdk_folder/sdk-build"

if [ ! -d "$sdk_folder" ] ; then
    echo_error "SDK Folder '$sdk_folder' does not exist"
    exit -1
fi

if [ -d "$sdk_build" ] ; then
    read -p "SDK build folder '$sdk_build' exists, overwrite? [y/n] " answer
    if [ "$answer" = "y" ] ; then
        echo_warn "Removing old '$sdk_build' folder"
        rm -r $sdk_build
        check_error "Failed to remove directory '$sdk_build'"
    else
        echo_error "Unabke to install patch, because the build folder exists"
        exit -1
    fi
fi

echo_info "Decompressing patch..."

export TMPDIR=`mktemp -d /tmp/selfextract.XXXXXX`

ARCHIVE=`awk '/^__ARCHIVE_BELOW__/ {print NR + 1; exit 0; }' $0`

tail -n+$ARCHIVE $0 | tar xz -C $TMPDIR
check_error "Failed to decompress the patch"

echo_info "Decompression finished"

echo_info "Installing patch"
mv $TMPDIR $sdk_build
check_error "Failed to install the patch"

exit 0

__ARCHIVE_BELOW__
