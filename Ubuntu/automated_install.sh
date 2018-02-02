#!/bin/bash

#-------------------------------------------------------
# Paste from developer.amazon.com below
#-------------------------------------------------------

# This is the name given to your device or mobile app in the Amazon developer portal. To look this up, navigate to https://developer.amazon.com/iba-sp/overview.html. It may be labeled Device Type ID.
# SDK_CONFIG_PRODUCT_ID=YOUR_PRODUCT_ID_HERE
SDK_CONFIG_PRODUCT_ID=

# Retrieve your client ID from the web settings tab within the developer console: https://developer.amazon.com/iba-sp/overview.html
# SDK_CONFIG_CLIENT_ID=YOUR_CLIENT_ID_HERE
SDK_CONFIG_CLIENT_ID=

# Retrieve your client secret from the web settings tab within the developer console: https://developer.amazon.com/iba-sp/overview.html
# SDK_CONFIG_CLIENT_SECRET=YOUR_CLIENT_SECRET_HERE
SDK_CONFIG_CLIENT_SECRET=

#-------------------------------------------------------
# No need to change anything below this...
#-------------------------------------------------------

#-------------------------------------------------------
# Pre-populated for testing. Feel free to change.
#-------------------------------------------------------

# Your Country. Must be 2 characters!
Country='US'
# Your state. Must be 2 or more characters.
State='CA'
# Your city. Cannot be blank.
City='San Diego'
# Your organization name/company name. Cannot be blank.
Organization='AVS_USER'
# Your device serial number. Cannot be blank, but can be any combination of characters.
SDK_CONFIG_DEVICE_SERIAL_NUMBER='123456789'
# Your KeyStorePassword. We recommend leaving this blank for testing.
KeyStorePassword=''
# Audio file locations. These should match their locations on the system.
SDK_SQLITE_DATABASE_FILE_PATH=''
SDK_ALARM_DEFAULT_SOUND_FILE_PATH=$(pwd)'/application-necessities/sound-files/med_system_alerts_melodic_01._TTH_.mp3'
SDK_ALARM_SHORT_SOUND_FILE_PATH=$(pwd)'/application-necessities/sound-files/med_system_alerts_melodic_01_short._TTH_.wav'
SDK_TIMER_DEFAULT_SOUND_FILE_PATH=$(pwd)'/application-necessities/sound-files/med_system_alerts_melodic_02._TTH_.mp3'
SDK_TIMER_SHORT_SOUND_FILE_PATH=$(pwd)'/application-necessities/sound-files/med_system_alerts_melodic_02_short._TTH_.wav'
# Default database location settings.
SDK_SQLITE_DATABASE_FILE_PATH=$(pwd)'/application-necessities/alerts.db'
SDK_SQLITE_SETTINGS_DATABASE_FILE_PATH=$(pwd)'/application-necessities/settings.db'
SDK_CERTIFIED_SENDER_DATABASE_FILE_PATH=$(pwd)'/application-necessities/certifiedSender.db'
SDK_NOTIFICATIONS_DATABASE_FILE_PATH=$(pwd)'/application-necessities/notifications.db'
# Default locale
SETTING_LOCALE_VALUE='en-US'

#-------------------------------------------------------
# Function to parse user's input.
#-------------------------------------------------------
# Arguments are: Yes-Enabled No-Enabled Quit-Enabled
YES_ANSWER=1
NO_ANSWER=2
QUIT_ANSWER=3
parse_user_input()
{
  if [ "$1" = "0" ] && [ "$2" = "0" ] && [ "$3" = "0" ]; then
    return
  fi
  while [ true ]; do
    Options="["
    if [ "$1" = "1" ]; then
      Options="${Options}y"
      if [ "$2" = "1" ] || [ "$3" = "1" ]; then
        Options="$Options/"
      fi
    fi
    if [ "$2" = "1" ]; then
      Options="${Options}n"
      if [ "$3" = "1" ]; then
        Options="$Options/"
      fi
    fi
    if [ "$3" = "1" ]; then
      Options="${Options}quit"
    fi
    Options="$Options]"
    read -p "$Options >> " USER_RESPONSE
    USER_RESPONSE=$(echo $USER_RESPONSE | awk '{print tolower($0)}')
    if [ "$USER_RESPONSE" = "y" ] && [ "$1" = "1" ]; then
      return $YES_ANSWER
    else
      if [ "$USER_RESPONSE" = "n" ] && [ "$2" = "1" ]; then
        return $NO_ANSWER
      else
        if [ "$USER_RESPONSE" = "quit" ] && [ "$3" = "1" ]; then
          printf "\nGoodbye.\n\n"
          exit
        fi
      fi
    fi
    printf "Please enter a valid response.\n"
  done
}

#----------------------------------------------------------------
# Function to select a user's preference between several options
#----------------------------------------------------------------
# Arguments are: result_var option1 option2...
select_option()
{
  local _result=$1
  local ARGS=("$@")
  if [ "$#" -gt 0 ]; then
      while [ true ]; do
         local count=1
         for option in "${ARGS[@]:1}"; do
            echo "$count) $option"
            ((count+=1))
         done
         echo ""
         local USER_RESPONSE
         read -p "Please select an option [1-$(($#-1))] " USER_RESPONSE
         case $USER_RESPONSE in
             ''|*[!0-9]*) echo "Please provide a valid number"
                          continue
                          ;;
             *) if [[ "$USER_RESPONSE" -gt 0 && $((USER_RESPONSE+1)) -le "$#" ]]; then
                    local SELECTION=${ARGS[($USER_RESPONSE)]}
                    echo "Selection: $SELECTION"
                    eval $_result=\$SELECTION
                    return
                else
                    clear
                    echo "Please select a valid option"
                fi
                ;;
         esac
      done
  fi
}

#-------------------------------------------------------
# Function to retrieve user account credentials
#-------------------------------------------------------
# Argument is: the expected length of user input
Credential=""
get_credential()
{
  Credential=""
  read -p ">> " Credential
  while [ "${#Credential}" -lt "$1" ]; do
    echo "Input has invalid length."
    echo "Please try again."
    read -p ">> " Credential
  done
}

#-------------------------------------------------------
# Function to confirm user credentials.
#-------------------------------------------------------
check_credentials()
{
  clear
  echo "======AVS User Credentials======"
  echo ""
  echo ""
  if [ "${#SDK_CONFIG_PRODUCT_ID}" -eq 0 ] || [ "${#SDK_CONFIG_CLIENT_ID}" -eq 0 ] || [ "${#SDK_CONFIG_CLIENT_SECRET}" -eq 0 ]; then
    echo "At least one of the needed credentials (Product ID, Client ID, or Client Secret) is missing."
    echo ""
    echo ""
    echo "These values can be found here https://developer.amazon.com/iba-sp/overview.html, fix this now?"
    echo ""
    echo ""
    parse_user_input 1 0 1
  fi

  # Print out of variables and validate user inputs
  if [ "${#SDK_CONFIG_PRODUCT_ID}" -ge 1 ] && [ "${#SDK_CONFIG_CLIENT_ID}" -ge 15 ] && [ "${#SDK_CONFIG_CLIENT_SECRET}" -ge 15 ]; then
    echo "Product ID >> $SDK_CONFIG_PRODUCT_ID"
    echo "Client ID >> $SDK_CONFIG_CLIENT_ID"
    echo "Client Secret >> $SDK_CONFIG_CLIENT_SECRET"
    echo ""
    echo ""
    echo "Is this information correct?"
    echo ""
    echo ""
    parse_user_input 1 1 0
    USER_RESPONSE=$?
    if [ "$USER_RESPONSE" = "$YES_ANSWER" ]; then
      return
    fi
  fi

  clear
  # Check SDK_CONFIG_PRODUCT_ID
  NeedUpdate=0
  echo ""
  if [ "${#SDK_CONFIG_PRODUCT_ID}" -eq 0 ]; then
    echo "Your Product ID is not set"
    NeedUpdate=1
  else
    echo "Your Product ID is set to: $SDK_CONFIG_PRODUCT_ID."
    echo "Is this information correct?"
    echo ""
    parse_user_input 1 1 0
    USER_RESPONSE=$?
    if [ "$USER_RESPONSE" = "$NO_ANSWER" ]; then
      NeedUpdate=1
    fi
  fi
  if [ $NeedUpdate -eq 1 ]; then
    echo ""
    echo "This value should match your Product ID (or Device Type ID) entered at https://developer.amazon.com/iba-sp/overview.html."
    echo "The information is located in the Products table under the Product ID column."
    echo "E.g.: Ubuntu"
    get_credential 1
    SDK_CONFIG_PRODUCT_ID=$Credential
  fi

  echo "-------------------------------"
  echo "ProductID is set to >> $SDK_CONFIG_PRODUCT_ID"
  echo "-------------------------------"

  # Check SDK_CONFIG_CLIENT_ID
  NeedUpdate=0
  echo ""
  if [ "${#SDK_CONFIG_CLIENT_ID}" -eq 0 ]; then
    echo "Your Client ID is not set"
    NeedUpdate=1
  else
    echo "Your Client ID is set to: $SDK_CONFIG_CLIENT_ID."
    echo "Is this information correct?"
    echo ""
    parse_user_input 1 1 0
    USER_RESPONSE=$?
    if [ "$USER_RESPONSE" = "$NO_ANSWER" ]; then
      NeedUpdate=1
    fi
  fi
  if [ $NeedUpdate -eq 1 ]; then
    echo ""
    echo "Please enter your Client ID."
    echo "This value should match the information at https://developer.amazon.com/iba-sp/overview.html."
    echo "The information is located under the 'Security Profile' tab."
    echo "E.g.: amzn1.application-oa2-client.xxxxxxxx"
    get_credential 28
    SDK_CONFIG_CLIENT_ID=$Credential
  fi

  echo "-------------------------------"
  echo "Client ID is set to >> $SDK_CONFIG_CLIENT_ID"
  echo "-------------------------------"

  # Check SDK_CONFIG_CLIENT_SECRET
  NeedUpdate=0
  echo ""
  if [ "${#SDK_CONFIG_CLIENT_SECRET}" -eq 0 ]; then
    echo "Your ClientSecret is not set"
    NeedUpdate=1
  else
    echo "Your ClientSecret is set to: $SDK_CONFIG_CLIENT_SECRET."
    echo "Is this information correct?"
    echo ""
    parse_user_input 1 1 0
    USER_RESPONSE=$?
    if [ "$USER_RESPONSE" = "$NO_ANSWER" ]; then
      NeedUpdate=1
    fi
  fi
  if [ $NeedUpdate -eq 1 ]; then
    echo ""
    echo "Please enter your Client Secret."
    echo "This value should match the information at https://developer.amazon.com/iba-sp/overview.html."
    echo "The information is located under the 'Security Profile' tab."
    echo "E.g.: fxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxa"
    get_credential 20
    SDK_CONFIG_CLIENT_SECRET=$Credential
  fi

  echo "-------------------------------"
  echo "Client Secret is set to >> $SDK_CONFIG_CLIENT_SECRET"
  echo "-------------------------------"

  check_credentials
}

#-------------------------------------------------------
# Inserts user-provided values into a template file
#-------------------------------------------------------
# Arguments are: template_directory, template_name, target_name
use_template()
{
  Template_Loc=$1
  Template_Name=$2
  Target_Name=$3
  while IFS='' read -r line || [[ -n "$line" ]]; do
    while [[ "$line" =~ (\$\{[a-zA-Z_][a-zA-Z_0-9]*\}) ]]; do
      LHS=${BASH_REMATCH[1]}
      RHS="$(eval echo "\"$LHS\"")"
      line=${line//$LHS/$RHS}
    done
    echo "$line" >> "$Template_Loc/$Target_Name"
  done < "$Template_Loc/$Template_Name"
}

clear

#--------------------------------------------------------------------------------------------
# Checking if script has been updated by the user with SDK_CONFIG_PRODUCT_ID,
# SDK_CONFIG_CLIENT_ID, and SDK_CONFIG_CLIENT_SECRET
#--------------------------------------------------------------------------------------------

if [ "$SDK_CONFIG_PRODUCT_ID" = "YOUR_PRODUCT_ID_HERE" ]; then
  SDK_CONFIG_PRODUCT_ID=""
fi
if [ "$SDK_CONFIG_CLIENT_ID" = "YOUR_CLIENT_ID_HERE" ]; then
  SDK_CONFIG_CLIENT_ID=""
fi
if [ "$SDK_CONFIG_CLIENT_SECRET" = "YOUR_CLIENT_SECRET_HERE" ]; then
  SDK_CONFIG_CLIENT_SECRET=""
fi

check_credentials

# Preconfigured variables
OS=rpi
User=$(id -un)
Group=$(id -gn)
Origin=$(pwd)
ConfigJSON=AlexaClientSDKConfig.json
TemplatePath=$Origin/avs-device-sdk-intel-speech-enabling-kit/Integration
TargetRelativePath=../../sdk-build/Integration

echo ""
echo ""
echo "==============================="
echo "********************************"
echo " *** STARTING INSTALLATION  ***"
echo "  **  WARNING: This script  **"
echo "   ** must be run locally! **"
echo "   ========================="
echo ""
echo ""


#echo "========== Configuring ALSA Devices =========="
#if [ -f /home/$User/.asoundrc ]; then
#  rm /home/$User/.asoundrc
#fi
#printf "pcm.!default {\n  type asym\n   playback.pcm {\n     type plug\n     slave.pcm \"hw:0,0\"\n   }\n   capture.pcm {\n     type plug\n     slave.pcm \"hw:1,0\"\n   }\n}" >> /home/$User/.asoundrc

# Fix template - missing '$' causes one variable to be missed
sed -i.bak 's/"{/"${/g' $TemplatePath/$ConfigJSON

# Create new configuration file from template and user-supplied information
echo -n "========== Generating "
echo -n $ConfigJSON
echo " =========="

if [ -f $TemplatePath/$TargetRelativePath/$ConfigJSON ]; then
   rm $TemplatePath/$TargetRelativePath/$ConfigJSON
fi
use_template $TemplatePath $ConfigJSON $TargetRelativePath/$ConfigJSON

echo ""
echo -n "========== Generated "
echo -n $ConfigJSON
echo " =========="
echo ""
echo ""
echo "==============================="
echo "The next step must be run locally"
echo "to authenticate with Amazon in a"
echo "web browser."
echo "==============================="
echo ""
echo "Continue and launch a web browser?"

parse_user_input 1 0 1

# Run the Authorization Server
echo -n "Web server starting...."
python $Origin/sdk-build/AuthServer/AuthServer.py &
pid=$!

# Launch a web browser to prompt for sign-in
python -mwebbrowser http://localhost:3000
echo "Launching web browser. Do not close this terminal window."

# Keep the terminal open until the AuthServer terminates on its own or this script ends
trap "kill $pid 2> /dev/null" EXIT
while kill -0 $pid 2> /dev/null; do
	sleep 2
done
trap - EXIT
