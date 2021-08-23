#!/bin/bash
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Call with <swarming dir> <top level of flutter checkout> <out_dir> [GN arguments...]

set -e
set -x

# export PATH="$1/recipe_bundle/depot_tools:${PATH}"
echo "LOOK AT OLD DEPOT_TOOLS"
ls -l $1/recipe_bundle/depot_tools
# echo "THIS IS THE GIT REV_PARSE HEAD"
#cd $1/recipe_bundle/depot_tools
#git rev-parse HEAD
# Checkpoint depot_tools in /tmp/
git clone https://chromium.googlesource.com/chromium/tools/depot_tools /tmp/depot_tools
# chmod 755 /tmp/depot_tools
export PATH="/tmp/depot_tools:${PATH}"
export VPYTHON_VIRTUALENV_ROOT=$1/cache/vpython
export CIPD_CACHE_DIR=/tmp/.cipd_cache
echo "LOOK AT ENV-"
env
echo "LOOK AT PATH-"
echo $PATH
echo "LOOK AT CONTENTS OF DEPOT_TOOLS"
ls -l /tmp/depot_tools
echo "WHICH VPYTHON3"
which vpython3

# e.g. /mnt/pd0/s/w/ir/cache/work/flutter/src
source_dir=$2
# e.g. /mnt/pd0/s/w/ir/cache/work/flutter/src/out/android_release
out_dir=$3
shift 3
cd $source_dir
flutter/tools/gn "$@"
# This is why we have to mount the entire swarming directory, GN sets a command
# to do a version check via relative path (../../flutter/third_party/gn/gn)
ninja -v -C $out_dir -j100
