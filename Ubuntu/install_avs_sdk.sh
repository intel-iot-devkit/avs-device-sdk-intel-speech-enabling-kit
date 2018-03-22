#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

sudo apt-get -f install

# get version 9 of nodejs
curl -sL https://deb.nodesource.com/setup_9.x | sudo -E bash -

sudo apt install bc python python-pip wget git gcc cmake build-essential \
libsqlite3-dev libcurl4-openssl-dev libfaad-dev libsoup2.4-dev libgcrypt20-dev \
libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly \
gstreamer1.0-plugins-bad libasound2-dev doxygen nghttp2 libnghttp2-dev libssl-dev \
redis-server nodejs

sudo pip install flask requests commentjson
# needed for display server
sudo npm install @angular/cli

PORT_AUDIO_TAR="pa_stable_v190600_20161030.tgz"
PORT_AUDIO_URL="http://www.portaudio.com/archives/$PORT_AUDIO_TAR"
CURL_DIR="curl-7.57.0"
CURL_TAR="$CURL_DIR.tar.gz"
CURL_URL="https://curl.haxx.se/download/$CURL_TAR"
BOOST_DIR="boost_1_66_0"
BOOST_VER="1.66.0"
BOOST_TAR="$BOOST_DIR.tar.gz"
BOOST_URL="https://dl.bintray.com/boostorg/release/$BOOST_VER/source/$BOOST_TAR"
BOOST_ROOT="/usr/local"

top="$(pwd)"
avs_top="$top/avs"

mkdir -p $avs_top
third_party_dir=$avs_top/third-party

mkdir -p $third_party_dir

if [[ ! -d $third_party_dir/portaudio ]]; then
cd $third_party_dir && wget $PORT_AUDIO_URL && tar xvf $PORT_AUDIO_TAR && cd portaudio && ./configure --with-jack=no && make -j4
fi

if [[ ! -d $third_party_dir/$CURL_DIR ]]; then
cd $third_party_dir && wget $CURL_URL && tar xvf $CURL_TAR && cd $CURL_DIR && ./configure --with-nghttp2 --prefix=/usr/local && make -j4 && sudo make install
fi

if [ ! -d $third_party_dir/$BOOST_DIR ]; then
    cd $third_party_dir
    wget $BOOST_URL
    tar -xvf $BOOST_TAR
    cd $BOOST_DIR
    # strap in, this takes a long time
    ./bootstrap.sh && sudo ./b2 install
fi

if [ ! -d $third_party_dir/socket.io-client-cpp ]; then
    cd $third_party_dir
    git clone --recurse-submodules https://github.com/socketio/socket.io-client-cpp.git
    cd socket.io-client-cpp/lib/websocketpp && git apply $avs_top/avs-device-sdk-intel-speech-enabling-kit/Ubuntu/websocketpp.patch
    cd $third_party_dir/socket.io-client-cpp
    cmake -DBOOST_ROOT:STRING=$BOOST_ROOT -DBOOST_VER:STRING=$BOOST_VER ./
    make -j4 && sudo make install
fi

if [ ! -d $third_party_dir/iot-server ]; then
    echo "Missing iot-server.tar.gz: no display server"
fi


cd $avs_top

pushd $avs_top/avs-device-sdk-intel-speech-enabling-kit
if [[ ! -d application-necessities ]]; then
tar xvf application-necessities.tar.gz -C $avs_top
fi
cp Ubuntu/automated_install.sh $avs_top
cat Ubuntu/startavs|sed "s#\\[AVSROOT\\]#$avs_top"#>$avs_top/startavs
chmod a+x $avs_top/startavs
popd

if [[ ! -d sdk-build ]]; then
mkdir sdk-build
fi

# call build script
$DIR/build.sh

echo "Please reboot system if running this script for the first time."
