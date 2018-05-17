#!/bin/bash
# The build script assumes to build on sdk-build folder one folder above the git project
#
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
THIRD_PARTY=$(cd $DIR/../third-party && pwd)
SDK_BUILD=$(cd $DIR/../sdk-build && pwd)
BOOST_ROOT="/usr/local"

echo "DIR $DIR"
echo "THIRD_PARTY $THIRD_PARTY"
echo "SDK_BUILD $SDK_BUILD"

if [[ ! -d $SDK_BUILD ]]; then
mkdir $SDK_BUILD
fi

pushd $SDK_BUILD
cmake ../avs-device-sdk-intel-speech-enabling-kit \
    -DCMAKE_PREFIX_PATH=/usr/local \
    -DCMAKE_BUILD_TYPE=DEBUG \
    -DHARDWARE_KEY_WORD_DETECTOR=ON \
    -DALSA_HARDWARE_CONTROLLER=ON \
    -DGSTREAMER_MEDIA_PLAYER=ON \
    -DPORTAUDIO=ON \
    -DPORTAUDIO_LIB_PATH=$THIRD_PARTY/portaudio/lib/.libs/libportaudio.a \
    -DPORTAUDIO_INCLUDE_DIR=$THIRD_PARTY/portaudio/include \
    -DSOCKETIO_INCLUDE_DIR=$THIRD_PARTY/socket.io-client-cpp/build/include \
    -DSOCKETIO_LIB_PATH=$THIRD_PARTY/socket.io-client-cpp/build/lib/Release/libsioclient.a \
    -DBOOST_LIB_PATH=$BOOST_ROOT/lib/libboost_system.a \
    -DSOCKETIO_DISPLAY_SERVER=ON

make -j4

popd

if [ $? -eq 0 ]
then
  echo "Successfully built AVS"
else
  echo "Build failed!!" >&2
  exit
fi
