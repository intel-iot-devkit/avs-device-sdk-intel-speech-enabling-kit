#!/bin/bash

GIT_REPO_URL=""
DRIVER_URL=""
PORT_AUDIO_URL="http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz"
PORT_AUDIO_TAR="pa_stable_v190600_20161030.tgz"

# Sound file URLS
SF_ALERT_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_02._TTH_.mp3"
SF_ALERT_SHORT_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_02_short._TTH_.wav"
SF_TIMER_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_01._TTH_.mp3"
SF_TIMER_SHORT_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_01_short._TTH_.wav"

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

function parse_user_input() {
    prompt="$1"
    yes="$2"
    no="$3"

    while true; do
        read -p "$prompt " answer

        if [ "$answer" == "$yes" ] ; then
            return $answer
        elif [ "$answer" == "$no" ] ; then
            return $answer
        else
            echo_error "Unknown selection: $answer"
        fi
    done
}

function verify_not_root() {
    if [[ $EUID -eq 0 ]]; then
        echo_error "Script must not be ran as root"
        exit -1
    fi
}

# Important directories and files
sdk_folder="$HOME/sdk-folder"
sdk_source="$sdk_folder/sdk-source"
sdk_build="$sdk_folder/sdk-build"

third_party="$sdk_folder/third-party"
git_repo="$sdk_source/avs-device-sdk"

portaudio="$third_party/portaudio"
portaudio_tar="$third_party/$PORT_AUDIO_TAR"
portaudio_lib="$portaudio/lib/.libs/libportaudio.a"
portaudio_include="$portaudio/include/"

app_necessities="$sdk_folder/application-necessities"
sound_files="$app_necessities/sound-files"
sf_alarm="$sound_files/med_system_alerts_melodic_01._TTH_.mp3"
sf_alarm_short="$sound_files/med_system_alerts_melodic_00_short._TTH_.wav"
sf_timer="$sound_files/med_system_alerts_melodic_02._TTH_.mp3"
sf_timer_short="$sound_files/med_system_alerts_melodic_02_short._TTH_.wav"

# Verify that the user is not running the script as root
verify_not_root

echo_info "Running apt update"
sudo apt update
check_error "Failed to update apt repositories"

echo_info "Installing dependencies"
sudo apt install -y \
    git \
    gcc \
    cmake \
    build-essential \
    libsqlite3-dev \
    libcurl4-openssl-dev \
    libfaad-dev \
    libsoup2.4-dev \
    libgcrypt20-dev \
    libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-good \
    libasound2-dev \
    doxygen
check_error "Failed to install of the dependencies"

# Creating the folder structure if it does not exist
if [ ! -d "$sdk_folder" ] ; then
    echo_info "Creating '$sdk_folder'"
    mkdir $sdk_folder
    check_error "Failed to create '$sdk_folder'"
fi

if [ ! -d "$sdk_source" ] ; then
    echo_info "Creating '$sdk_source'"
    mkdir $sdk_source
    check_error "Failed to create '$sdk_source'"
fi

if [ ! -d "$sdk_build" ] ; then
    echo_info "Creating '$sdk_build'"
    mkdir $sdk_build
    check_error "Failed to create '$sdk_build'"
fi

if [ ! -d "$third_party" ] ; then
    echo_info "Creating '$third_party'"
    mkdir $third_party
    check_error "Failed to create '$third_party'"
fi

# Clone the gut repository if it does not exist, or if the directory is not a
# git repository
if [ ! -d "$git_repo" ] || [ `git -C $git_repo rev-parse` -ne 0 ] ; then
    if [ -d "$git_repo" ] ; then
        while true; do
            read -p "The git repository directory already exists, is it ok to delete it? (y/n)" answer
            if [ "$answer" == "y" ] || [ "$answer" == "Y" ] ; then
                echo_info "Deleting '$git_repo'"
                rm -r $git_repo
                check_error "Failed to delete '$git_repo'"
                break
            elif [ "$answer" == "n" ] || [ "$answer" == "N" ] ; then
                echo_error "Installation failed due to '$git_repo' already existing and not being a git repository"
                exit -1
            else
                echo_error "Unknown user input: $answer"
            fi
        done
    fi

    echo_info "Cloning git repository '$GIT_REPO_URL' to '$git_repo'"
    git clone $GIT_REPO_URL $git_repo
    check_error "Failed to clone '$GIT_REPO_URL'"
fi

# TODO: Install the driver

# Installing third-party dependencies
if [ ! -f "$libportaudio" ] ; then
    pushd $PWD  # Save current directory
    
    cd $third_party
    if [ ! -f "$portaudio_tar" ] ; then
        echo_info "Downloading PortAudio"
        wget $PORT_AUDIO_URL
        check_error "Failed to download PortAudio from '$PORT_AUDIO_URL'"
    fi

    if [ ! -d "$portaudio" ] ; then
        echo_info "Extracting PortAudio"
        tar xvf $portaudio_tar
        check_error "Failed to extract PortAudio from '$portaudio_tar'"
    fi
    
    pushd $PWD
    cd $portaudio
    echo_info "Compiling PortAudio"
    ./configure
    check_error "Failed to configure PortAudio for compilation"
    make
    check_error "Failed to compile PortAudio"

    popd  # Return to previous directory
fi

if [ ! -d "$app_necessities" ] ; then
    echo_info "Creating '$app_necessities'"
    mkdir $app_necessities
    check_error "Failed to create '$app_necessities'"
fi

if [ ! -d "$sound_files" ] ; then
    echo_info "Creating '$sound_files'"
    mkdir $sound_files
    check_error "Failed to create '$app_necessities'"
fi

# Downloading sound files
pushd $PWD
cd $sound_files

if [ ! -f "$sf_alarm" ] ; then
    echo_info "Downloading sound file '$SF_ALERT_URL'"
    wget -c $SF_ALERT_URL
    check_error "Failed to download sound file '$SF_ALERT_URL'"
fi

if [ ! -f "$sf_alarm_short" ] ; then
    echo_info "Downloading sound file '$SF_ALERT_SHORT_URL'"
    wget -c $SF_ALERT_SHORT_URL
    check_error "Failed to download sound file '$SF_ALERT_SHORT_URL'"
fi

if [ ! -f "$sf_timer" ] ; then
    echo_info "Downloading sound file '$SF_TIMER_URL'"
    wget -c $SF_TIMER_URL
    check_error "Failed to download sound file '$SF_TIMER_URL'"
fi

if [ ! -f "$sf_timer_short" ] ; then
    echo_info "Downloading sound file '$SF_TIMER_SHORT_URL'"
    wget -c $SF_TIMER_SHORT_URL
    check_error "Failed to download sound file '$SF_TIMER_SHORT_URL'"
fi

popd

# Get the account specific information from the user
product_id=""
client_id=""
client_secret=""

while true ; do
    read -p "Please enter your Product ID: " product_id
    read -p "Please enter your Client ID: " client_id
    read -p "Please enter your Client Secret: " client_secret
    
    echo -e "Are these values correct?\n"\
        "Product ID: $product_id\n"\
        "Client ID: $client_id\n"\
        "Client Secret: $client_secret"

    answer=`parse_user_input "(y/n)" "y" "n"`

    if [ "$answer" == "y" ] ; then
        break
    fi
done

# Compile the code
pushd $PWD
cd $sdk_build

echo_info "Building the C++ SDK"
cmake -DCMAKE_BUILD_TYPE=DEBUG \
    -DHARDWARE_KEY_WORD_DETECTOR=ON \
    -DALSA_HARDWARE_CONTROLLER=ON \
    -DGSTREAMER_MEDIA_PLAYER=ON \
    -DSTRESS_TESTER_APP=ON \
    -DPORTAUDIO=ON \
    -DPORTAUDIO_LIB_PATH=$portaudio_lib \
    -DPORTAUDIO_INCLUDE_DIR=$portaudio_include \
    $git_repo
check_error "CMake failed for building C++ SDK"
make SampleApp -j2
check_error "Failed to compile C++ SDK"

popd

# TODO: Setup the JSON file

# TODO: Run authentication server
