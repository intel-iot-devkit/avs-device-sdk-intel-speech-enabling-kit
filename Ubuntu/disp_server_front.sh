#!/bin/bash

sudo apt install ng-common

firefox -new-tab http://localhost:3001 &
#run from the avs top
cd third-party/iot-server/ui-generator && ng build --outputPath ../public --watch
