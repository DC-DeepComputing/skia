#!/bin/bash
./pushit.sh
adb shell /data/local/tmp/perf-setup.sh
adb shell /data/local/tmp/perfetto.sh
adb pull /data/misc/perfetto-traces/nanobench.perfetto-trace .