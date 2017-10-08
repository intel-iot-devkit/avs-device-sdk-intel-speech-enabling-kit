#!/bin/bash

cp ./AlexaClientSDKConfig.json ./avs_build/Integration/
cd avs_build
python AuthServer/AuthServer.py &
sleep 3
google-chrome http://localhost:3000
