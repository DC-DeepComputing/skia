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
--gpuStatsDump true \
--ms 3000 \
--scales 1.0 \
--config gles 8888 \
--match mobi_wiki \
--skps /data/local/tmp/skp \
--key arch arm64 compiler Clang cpu_or_gpu GPU cpu_or_gpu_value Adreno640 extra_config Android model Pixel4 os Android \
--properties gitHash 3aaed99930f1c913255fe995df17c77a863e1ab3 swarming_bot_id skia-rpi2-rack4-shelf3-007 swarming_task_id 572f3f40438e7f11 \
-i /data/local/tmp/resources
echo $? >/data/local/tmp/rc

kill $PID