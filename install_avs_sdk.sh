#!/bin/bash

HOME="/home/${SUDO_USER}"

GIT_REPO_URL="https://github.com/intel-iot-devkit/avs-device-sdk-intel-speech-enabling-kit.git"
GIT_DRIVER_URL="https://github.com/intel-iot-devkit/alsa-driver-intel-speech-enabling-kit.git"
PORT_AUDIO_URL="http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz"
PORT_AUDIO_TAR="pa_stable_v190600_20161030.tgz"
CONFIG_JSON="AlexaClientSDKConfig.json"
CONFIG_JSON_NO_SCRATCH=${CONFIG_JSON}."template"
PRE_BUILT_BIN_URL="https://downloadmirror.intel.com/27345/eng/prebuilt.tar.gz"

# Putting the pre-built tar file into /tmp/ so it is deleted on reboot
PRE_BUILT_DEST="/tmp/prebuilt.tar.gz"

# Temporary configuration file which will store prior account information values
# given by the users. Note that it is in the /tmp/ directory so that when the
# system reboots the cache is erased.
CONFIG_CACHE_LOC="/tmp/amazon_config.txt"

# Sound file URLS
SF_ALERT_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_02._TTH_.mp3"
SF_ALERT_SHORT_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_02_short._TTH_.wav"
SF_TIMER_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_01._TTH_.mp3"
SF_TIMER_SHORT_URL="https://images-na.ssl-images-amazon.com/images/G/01/mobile-apps/dex/alexa/alexa-voice-service/docs/audio/states/med_system_alerts_melodic_01_short._TTH_.wav"
SETTING_LOCALE_VALUE='en-US'

## Logging 
RED='\033[0;31m'
YELLOW="\033[1;33m"
GREEN="\033[0;32m"
NC='\033[0m' # No Color

function echo_info() {
    echo -e "${GREEN}`date` : INFO : $1 ${NC}"
}

function echo_warn() {
    echo -e "${YELLOW}`date` : WARN : $1 ${NC}"
}

function echo_error() {
    echo -e "${RED}`date` : ERROR : $1 ${NC}"
}

function echo_fatal() {
    echo -e "${RED}`date` : FATAL : $1 ${NC}"
    exit -1
}

function check_error() {
    if [ $? -ne 0 ] ; then
        echo_fatal "$1"
    fi
}

function parse_user_input() {
    local prompt="$1"
    local yes="$2"
    local no="$3"
    local _answer=$4

    while true; do
        read -p "$prompt " answer

        if [ "$answer" == "$yes" ] || [ "$answer" == "$no" ] ; then
            eval $_answer="$answer"
            break
        else
            echo_error "Unknown selection: $answer"
        fi
    done
}

function verify_root() {
    if [[ $EUID -ne 0 ]]; then
        echo_fatal "Script must be ran as root"
    fi
}

function usage() {
    echo "usage $1 [-h|--help] [options]"
    echo -e "\t-h|--help      : Show this help"
    echo -e "\t--debug        : Compile the AVS SDK mode in debug mode"
    echo -e "\t--from-scratch : Setup the AVS SDK from scratch"
    echo -e "\t--use-prebuilt : Download and use pre-built binaries"
    exit 0
}

# Important directories and files
sdk_folder="$HOME/sdk-folder"
sdk_source="$sdk_folder/sdk-source"
sdk_build="$sdk_folder/sdk-build"

third_party="$sdk_folder/third-party"
git_repo="$sdk_source/avs-device-sdk"

# Driver file directories
driver_repo="$sdk_folder/driver-repo"

portaudio="$third_party/portaudio"
portaudio_tar="$third_party/$PORT_AUDIO_TAR"
portaudio_lib="$portaudio/lib/.libs/libportaudio.a"
portaudio_include="$portaudio/include/"

app_necessities="$sdk_folder/application-necessities"
sound_files="$app_necessities/sound-files"

startsample_script="$sdk_folder/startsample.sh"

# Asound config file
ASOUND_CONFIG_FILE="$git_repo/asound.conf"
ASOUND_CONFIG_FILE_PRE_BUILT="$sdk_folder/asound.conf"

# Your device serial number. Cannot be blank, but can be any combination of characters.
SDK_CONFIG_DEVICE_SERIAL_NUMBER='123456789'
# Audio file locations. These should match their locations on the system.
SDK_SQLITE_DATABASE_FILE_PATH=''
SDK_ALARM_DEFAULT_SOUND_FILE_PATH="$sound_files/med_system_alerts_melodic_01._TTH_.mp3"
SDK_ALARM_SHORT_SOUND_FILE_PATH="$sound_files/med_system_alerts_melodic_01_short._TTH_.wav"
SDK_TIMER_DEFAULT_SOUND_FILE_PATH="$sound_files/med_system_alerts_melodic_02._TTH_.mp3"
SDK_TIMER_SHORT_SOUND_FILE_PATH="$sound_files/med_system_alerts_melodic_02_short._TTH_.wav"
# Default database location settings.
SDK_SQLITE_DATABASE_FILE_PATH="$app_necessities/alerts.db"
SDK_SQLITE_SETTINGS_DATABASE_FILE_PATH="$app_necessities/settings.db"
SDK_CERTIFIED_SENDER_DATABASE_FILE_PATH="$app_necessities/certifiedSender.db"

config_template="$git_repo/Integration/$CONFIG_JSON"
config_template_no_scratch="$sdk_build/Integration/$CONFIG_JSON_NO_SCRATCH"
config_dest="$sdk_build/Integration/$CONFIG_JSON"


# Get the account specific information from the user
SDK_CONFIG_PRODUCT_ID=""
SDK_CONFIG_CLIENT_ID=""
SDK_CONFIG_CLIENT_SECRET=""
from_scratch=0
compile_sdk_debug=0
use_prebuilt=0

function generate_json_config() {
    if [[ $from_scratch -eq 0 ]] ; then
        config_template=$config_template_no_scratch
    fi
    # Generate the JSON configuration
    # Fix template - missing '$' causes one variable to be missed
    echo_info "Generating '$config_dest'"
    sed -i.bak 's/"{/"${/g' $config_template 

    if [ -f "$config_dest" ] ; then
        echo_warn "Deleting old JSON config '$config_dest'"
        rm $config_dest
        check_error "Failed to remove the old JSON config"
    fi

    #-------------------------------------------------------
    # Inserts user-provided values into a template file
    #-------------------------------------------------------
    # Arguments are: template path, target path 
    function use_template() {
      template=$1
      dest=$2
      while IFS='' read -r line || [[ -n "$line" ]]; do
        while [[ "$line" =~ (\$\{[a-zA-Z_][a-zA-Z_0-9]*\}) ]]; do
          LHS=${BASH_REMATCH[1]}
          RHS="$(eval echo "\"$LHS\"")"
          line=${line//$LHS/$RHS}
        done
        echo "$line" >> "$dest"
      done < "$template"
    }

    use_template $config_template $config_dest
	
    if [[ $from_scratch -eq 1 ]] ; then
        echo_info "Copying the template to $sdk_folder"
        cp $config_template $config_template_no_scratch
    fi
    
    echo_info "Changing ownership of $sdk_folder to $SUDO_USER"
    chown -R ${SUDO_USER}:${SUDO_USER} $sdk_folder
    check_error "Failed to transfer ownership"

    # Starting web server
    echo_info "Starting authentication web server"
    python $sdk_build/AuthServer/AuthServer.py &
    pid=$!
    check_error "Failed to start the authentication web server"

    #echo_info "Launching web browser"
    #python -mwebbrowser http://localhost:3000
    echo_info "Open up localhost:3000 in your chromium browser to complete the setup"
   
    # Keep script running until the AuthServer terminates 
    trap "kill $pid 2> /dev/null" EXIT
    while kill -0 $pid 2> /dev/null; do
        sleep 2
    done
    trap - EXIT
}

function get_account_info() {
    echo "Please provide your Amazon account specific information"
    while true ; do
        read -p "Please enter your Product ID: " SDK_CONFIG_PRODUCT_ID
        read -p "Please enter your Client ID: " SDK_CONFIG_CLIENT_ID
        read -p "Please enter your Client Secret: " SDK_CONFIG_CLIENT_SECRET
        
        echo -e "Are these values correct?\n"\
            "Product ID: $SDK_CONFIG_PRODUCT_ID\n"\
            "Client ID: $SDK_CONFIG_CLIENT_ID\n"\
            "Client Secret: $SDK_CONFIG_CLIENT_SECRET"

        parse_user_input "(y/n)" "y" "n" answer

        if [ "$answer" == "y" ] ; then
            # generating cache
            echo -n "$SDK_CONFIG_PRODUCT_ID " > $CONFIG_CACHE_LOC
            echo -n "$SDK_CONFIG_CLIENT_ID " >> $CONFIG_CACHE_LOC
            echo "$SDK_CONFIG_CLIENT_SECRET" >> $CONFIG_CACHE_LOC
            break
        fi
    done
}

function install_kernel_from_dir() {
    dir="$1"
    if [ ! -d "$dir" ] ; then
        echo_fatal "Attempting to install the kernel from a directory that does not exist: '$dir'"
    fi

    pushd $PWD
    cd $dir

    echo_warn "Removing old '/lib/modules'"
    rm -r /lib/modules
    check_error "Failed to remove old '/lib/modules'"

    echo_info "Installing new '/lib/modules'"
    cp -r ./lib/modules/ /lib/modules
    check_error "Failed to install new '/lib/modules'"

    echo_info "Copying over new dtb files"
    cp *.dtb /boot/
    check_error "Failed to copy over the new dtb files"

    echo_info "Copying over the dtb overlays"
    cp overlays/*.dtb* /boot/overlays/
    check_error "Failed to copy over the dtb overlays"

    echo_info "Copying over the dtb overlays README"
    cp overlays/README /boot/overlays/
    check_error "Fauled to copy over the dtb overlays README"

    echo_info "Copying over the new kernel7.img"
    cp ./kernel7.img /boot/
    check_error "Failed to copy over the new kernel7.img"

    popd
}

function add_modules() {
    if [ $(grep -c "snd_soc_s1000" /etc/modules) -eq 0 ]; then
        echo "snd_soc_s1000" >> /etc/modules
        echo "snd_soc_s1000_mach" >> /etc/modules
    fi
}

function set_spi_on() {
    echo_info "Setting the spi=on in /boot/config.txt"
    sed -i -e 's/#dtparam=spi=on/ dtparam=spi=on/' /boot/config.txt
    check_error "Failed to set spi=on"
}

function disable_default_audio() {
    echo_info "Disabling the default audio cards in /boot/config.txt"
    sed -i -e 's/dtparam=audio=on/ dtparam=audio=off/' /boot/config.txt
    check_error "Failed to diable the default audio cards"
}


# Verify that the user is not running the script as root
verify_root

for var in "$@" ; do
    case "$var" in
        "--from-scratch" )
            echo_info "Running installation from scratch"
            if [[ $use_prebuilt -eq 1 ]] ; then
                echo_fatal "Incompatible parameters: cannot use pre-built and build from scratch"
            fi
            from_scratch=1 ;;
        "--use-prebuilt" )
            echo_info "Using pre-built binaries"
            if [[ $from_scratch -eq 1 ]] ; then
                echo_fatal "Incompatible parameters: cannot use pre-built and build from scratch"
            fi
            use_prebuilt=1 ;;
        "--debug" )
            echo_info "Compiling SDK in debug mode"
            compile_sdk_debug=1 ;;
        "-h" )
            usage $0 ;;
        "--help" )
            usage $0 ;;
    esac
done

if [ -f "$CONFIG_CACHE_LOC" ] ; then
    echo_info "Reading Amazon account info cache"
    read -r SDK_CONFIG_PRODUCT_ID SDK_CONFIG_CLIENT_ID SDK_CONFIG_CLIENT_SECRET < "$CONFIG_CACHE_LOC"
    check_error "Failed to read Amazon account info cache"

    echo -e "The following values were found for your Amazon account," \
        "are they correct?\n"\
        "Product ID: $SDK_CONFIG_PRODUCT_ID\n"\
        "Client ID: $SDK_CONFIG_CLIENT_ID\n"\
        "Client Secret: $SDK_CONFIG_CLIENT_SECRET"

    parse_user_input "(y/n)" "y" "n" answer
    
    if [ "$answer" == "n" ] ; then
        echo_info "Obtaining Amazon developer account information for the device"
        get_account_info
    fi
else
    # The script cannot find a prior cache with the account info, getting the
    # account information from the user
    echo_info "Obtaining Amazon developer account information for the device"
    get_account_info
fi

# If we are not running from scratch, then generate the JSON configuration file
# and then exit the script
if [[ $from_scratch -eq 0 ]] && [[ $use_prebuilt -ne 1 ]] ; then
    generate_json_config
    exit 0
fi

echo_info "Running apt update"
apt update
check_error "Failed to update apt repositories"

echo_info "Installing dependencies"
apt install -y \
    bc \
    python \
    python-pip \
    wget \
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

echo_info "Installing Python dependencies"
pip install flask requests
check_error "Failed to install Python dependencies"

if [[ $use_prebuilt -eq 1 ]] ; then
    echo_info "Downloading pre-built binaries"
    wget $PRE_BUILT_BIN_URL -O $PRE_BUILT_DEST
    check_error "Failed to download pre-build binaries from '$PRE_BUILT_BIN_URL'"

    echo_info "Extracting pre-built binaries"
    tar xvf $PRE_BUILT_DEST
    check_error "Failed to extract the pre-built binaries"

    echo_info "Installing kernel"
    install_kernel_from_dir "$sdk_folder/kernel-build"

    echo_warn "Removing '$sdk_folder/kernel-build"
    rm -r $sdk_folder/kernel-build
    check_error "Failed to remove '$sdk_folder'"
    
    # Copy the asound file
    echo_info "Copying the suecreek asound.conf file"
    cp "$ASOUND_CONFIG_FILE_PRE_BUILT" /etc/asound.conf
    check_error "Failed to copy the asound.conf"
    
    # Removing the asound.conf that is included in the pre-built binaries 
    # because it is not needed at any point after this
    echo_warn "Removing '$ASOUND_CONFIG_FILE_PRE_BUILT'"
    rm $ASOUND_CONFIG_FILE_PRE_BUILT
    check_error "Faild to remove '$ASOUND_CONFIG_FILE_PRE_BUILT'"

    set_spi_on
    disable_default_audio
    add_modules

    generate_json_config
    exit 0
fi

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
        echo_warn "Deleting '$git_repo'"
        rm -r $git_repo
        check_error "Failed to delete '$git_repo'"
        break
    fi

    echo_info "Cloning git repository '$GIT_REPO_URL' to '$git_repo'"
    git clone $GIT_REPO_URL $git_repo
    check_error "Failed to clone '$GIT_REPO_URL'"
fi

# Compile/install the driver

pushd $PWD
cd $sdk_folder

# Clone the driver git repository if it does not exist, or if the directory is 
# not a git repository
if [ ! -d "$driver_repo" ] || [ `git -C $driver_repo rev-parse` -ne 0 ] ; then
    if [ -d "$driver_repo" ] ; then
        echo_warn "Deleting '$driver_repo'"
        rm -r $driver_repo
        check_error "Failed to delete '$driver_repo'"
        break
    fi

    echo_info "Cloning git repository '$GIT_DRIVER_URL' to '$driver_repo'"
    git clone $GIT_DRIVER_URL $driver_repo
    check_error "Failed to clone '$GIT_DRIVER_URL'"
fi

cd $driver_repo

# The assumption here is that if we did not get to the last step, then recompile
# the kernel
if [ ! -f "./kernel7.img" ] ; then
    echo_info "Clean the kernel tree"
    make mrproper
    check_error "Failed to clean the kernel tree"
    
    echo_info "Making driver .config"
    make intel_s1000_defconfig
    check_error "Failed to make .config"

    echo_info "Compiling kernel - this may take awhile, go get coffee"
    make -j2 zImage modules dtbs
    check_error "Failed to compile the kernel"

    echo_info "Installing new /lib/modules"
    make modules_install
    check_error "Failed to install new /lib/modules"

    echo_info "Copying over new dtb files"
    cp arch/arm/boot/dts/*.dtb /boot/
    check_error "Failed to copy over the new dtb files"

    echo_info "Copying over the dtb overlays"
    cp arch/arm/boot/dts/overlays/*.dtb* /boot/overlays/
    check_error "Failed to copy over the dtb overlays"

    echo_info "Copying over the dtb overlays README"
    cp arch/arm/boot/dts/overlays/README /boot/overlays/
    check_error "Fauled to copy over the dtb overlays README"
    
    # Generating the kernel7.img file in the current directory to make sure the
    # command succeeds before replacing the /boot/kernel7.img to make sure that
    # it succeeds before possibly messing up the system.
    echo_info "Creating over the new kernel7.img"
    ./scripts/mkknlimg arch/arm/boot/zImage ./kernel7.img
    check_error "Failed to create the new kernel7.img"

    echo_info "Copying over the new kernel7.img"
    cp ./kernel7.img /boot/
    check_error "Failed to copy over the new kernel7.img"
fi

popd

set_spi_on
disable_default_audio

# Installing third-party dependencies
if [ ! -f "$portaudio_lib" ] ; then
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

    popd
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

if [ ! -f "$SDK_ALARM_DEFAULT_SOUND_FILE_PATH" ] || \
    [ ! -f "$SDK_ALARM_SHORT_SOUND_FILE_PATH" ] || \
    [ ! -f "$SDK_TIMER_DEFAULT_SOUND_FILE_PATH" ] || \
    [ ! -f "$SDK_TIMER_SHORT_SOUND_FILE_PATH" ] ; then
    echo_info "Downloading alarm and timer sound files"
    wget -c $SF_ALERT_URL && wget -c $SF_ALERT_SHORT_URL && wget -c $SF_TIMER_URL && wget -c $SF_TIMER_SHORT_URL
    check_error "Failed to download sound file '$SF_ALERT_URL'"
fi

popd

# Compile the code
pushd $PWD
cd $sdk_build

echo_info "Building the C++ SDK"
if [[ $compile_sdk_debug == 1 ]] ; then
    cmake -DCMAKE_BUILD_TYPE=DEBUG \
        -DHARDWARE_KEY_WORD_DETECTOR=ON \
        -DALSA_HARDWARE_CONTROLLER=ON \
        -DGSTREAMER_MEDIA_PLAYER=ON \
        -DPORTAUDIO=ON \
        -DPORTAUDIO_LIB_PATH=$portaudio_lib \
        -DPORTAUDIO_INCLUDE_DIR=$portaudio_include \
        $git_repo
else
    cmake -DHARDWARE_KEY_WORD_DETECTOR=ON \
        -DALSA_HARDWARE_CONTROLLER=ON \
        -DGSTREAMER_MEDIA_PLAYER=ON \
        -DPORTAUDIO=ON \
        -DPORTAUDIO_LIB_PATH=$portaudio_lib \
        -DPORTAUDIO_INCLUDE_DIR=$portaudio_include \
        $git_repo
fi
check_error "CMake failed for building C++ SDK"
make SampleApp -j2
check_error "Failed to compile C++ SDK"

popd

# Copy the asound file
echo_info "Copying the suecreek asound.conf file"
cp "$ASOUND_CONFIG_FILE" /etc/asound.conf
check_error "Failed to copy the asound.conf"

# Generating start script
echo_info "Generating '$startsample_script'"
if [ -f "$startsample_script" ] ; then
    echo_warn "Deleting old '$startsample_script'"
    rm $startsample_script
    check_error "Failed to delete the old '$startsample_script'"
fi

echo "#!/bin/bash" > $startsample_script
echo "cd $sdk_build/SampleApp/src/" >> $startsample_script
echo "TZ=UTC ./SampleApp $config_dest" >> $startsample_script

add_modules

generate_json_config
echo_warn "Please reboot your system"
