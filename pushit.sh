#!/bin/bash

adb push config.pbtx /data/local/tmp/config.pbtx
adb push perfetto.sh /data/local/tmp/perfetto.sh
adb push perf-setup.sh /data/local/tmp/perf-setup.sh
adb shell chmod +x /data/local/tmp/perfetto.sh