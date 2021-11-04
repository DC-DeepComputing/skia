#!/bin/bash
adb shell /data/local/tmp/perfetto.sh
adb pull /data/misc/perfetto-traces/nanobench.perfetto-trace .