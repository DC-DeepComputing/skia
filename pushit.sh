#!/bin/bash

SKP=/tmp/skp
if [ ! -d "$SKP" ]; then
./bin/sk asset download --ci skp $SKP
fi

#adb push --sync `readlink -f /tmp/skp/*` /data/local/tmp/skp/
adb push resources /data/local/tmp/resources/
adb push config.pbtx /data/local/tmp/config.pbtx
adb push perfetto.sh /data/local/tmp/perfetto.sh
adb push perf-setup.sh /data/local/tmp/perf-setup.sh
adb shell chmod +x /data/local/tmp/perfetto.sh