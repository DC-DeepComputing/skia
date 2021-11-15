#!/bin/bash

NDK=/tmp/ndk
if [ ! -d "$NDK" ]; then 
./bin/sk asset download android_ndk_linux $NDK
fi

./bin/gn gen out/arm64 --args='is_debug=false ndk="/tmp/ndk" target_cpu="arm64"'
ninja -C out/arm64 nanobench
adb push out/arm64/nanobench /data/local/tmp/nanobench
