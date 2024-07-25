#!/bin/bash
rm -rf release
mkdir -p release

cp -rf HapticFloor *.{hpp,cpp,txt,json} LICENSE release/

mv release score-addon-hapticfloor
7z a score-addon-hapticfloor.zip score-addon-hapticfloor
