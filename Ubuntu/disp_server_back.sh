#!/bin/bash

#run from the avs top
cd third-party/iot-server && npm install && DEBUG=* PORT=3001 SOCKETIOS=true npm start
