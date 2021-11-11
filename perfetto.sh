# Copyright 2021 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x;
setprop persist.traced.enable 1
setprop debug.egl.traceGpuCompletion 1

PID=$(cat /data/local/tmp/config.pbtx | perfetto  --background -c - --txt -o /data/misc/perfetto-traces/nanobench.perfetto-trace)

#LD_LIBRARY_PATH=/data/local/tmp/ /data/local/tmp/dm --src gm --nocpu --config gles --match gpu_blur_utils_ref --writePath /data/local/tmp/dm_out

LD_LIBRARY_PATH=/data/local/tmp/ \
/data/local/tmp/nanobench \
--pre_log \
--verbose \
--gpuStatsDump true \
--ms 1300 \
--scales 1.0 \
--config gles 8888 \
--match mobi_ \
--skps /data/local/tmp/skp \
-i /data/local/tmp/resources
echo $? >/data/local/tmp/rc

kill $PID