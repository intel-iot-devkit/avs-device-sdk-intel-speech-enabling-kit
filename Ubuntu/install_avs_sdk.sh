#!/bin/bash

sudo apt-get -f install

sudo apt install bc python python-pip wget git gcc cmake build-essential \
libsqlite3-dev libcurl4-openssl-dev libfaad-dev libsoup2.4-dev libgcrypt20-dev \
libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly \
libasound2-dev doxygen nghttp2 libnghttp2-dev libssl-dev

sudo pip install flask requests commentjson

PORT_AUDIO_TAR="pa_stable_v190600_20161030.tgz"
PORT_AUDIO_URL="http://www.portaudio.com/archives/$PORT_AUDIO_TAR"
CURL_DIR="curl-7.57.0"
CURL_TAR="$CURL_DIR.tar.gz"
CURL_URL="https://curl.haxx.se/download/$CURL_TAR"

top="$(pwd)"
avs_top="$top/avs"

mkdir -p $avs_top
third_party_dir=$avs_top/third-party

mkdir -p $third_party_dir

if [[ ! -d $third_party_dir/portaudio ]]; then
cd $third_party_dir && wget $PORT_AUDIO_URL && tar xvf $PORT_AUDIO_TAR && cd portaudio && ./configure have_jack=no && make -j4
fi

if [[ ! -d $third_party_dir/$CURL_DIR ]]; then
cd $third_party_dir && wget $CURL_URL && tar xvf $CURL_TAR && cd $CURL_DIR && ./configure --with-nghttp2 --prefix=/usr/local && make -j4 && sudo make install
fi

cd $avs_top

pushd $avs_top/avs-device-sdk-intel-speech-enabling-kit
if [[ ! -d application-necessities ]]; then
tar xvf application-necessities.tar.gz -C $avs_top
fi
cp Ubuntu/automated_install.sh $avs_top
cat Ubuntu/startavs|sed "s#\\[AVSROOT\\]#$top"#>$top/startavs
chmod a+x $top/startavs
popd

if [[ ! -d sdk-build ]]; then
mkdir sdk-build
fi

cd sdk-build && cmake ../avs-device-sdk-intel-speech-enabling-kit \
    -DCMAKE_PREFIX_PATH=/usr/local \
    -DCMAKE_BUILD_TYPE=DEBUG \
    -DHARDWARE_KEY_WORD_DETECTOR=ON \
    -DALSA_HARDWARE_CONTROLLER=ON \
    -DGSTREAMER_MEDIA_PLAYER=ON \
    -DPORTAUDIO=ON \
    -DPORTAUDIO_LIB_PATH=$third_party_dir/portaudio/lib/.libs/libportaudio.a \
    -DPORTAUDIO_INCLUDE_DIR=$third_party_dir/portaudio/include \
    -DEMIT_RAW_JSON_PAYLOAD=ON

make -j4

if [ $? -eq 0 ]
then
  echo "Successfully built AVS"
else
  echo "Build failed!!" >&2
  exit
fi

cd $avs_top && ./automated_install.sh

