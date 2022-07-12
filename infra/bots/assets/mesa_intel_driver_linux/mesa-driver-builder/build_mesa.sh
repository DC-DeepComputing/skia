#! /bin/bash
#
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Builds the mesa driver and copies it to /OUT
# This script uses the environement variable $MESA_VERSION
# to dermine which version to download and build.

set -ex

pushd /tmp

curl  https://archive.mesa3d.org//mesa-${MESA_VERSION}.tar.xz -o mesa.tar.xz
tar xf mesa.tar.xz
cd mesa-$MESA_VERSION/

ls -al

meson build/
meson configure --prefix="${PWD}/build/install" build/ -Ddri-drivers-path=.:../../:../../dri
ninja -C build/
ninja -C build/ install
rsync -av build/install/ /OUT

# Change "library_path" to "./lib/x86_64-linux-gnu/libvulkan_intel.so" and write it out
# in the root of the /OUT directory.
jq <  build/install/share/vulkan/icd.d/intel_icd.x86_64.json '.ICD.library_path="./lib/x86_64-linux-gnu/libvulkan_intel.so"' >  /OUT/intel_icd.x86_64.json

popd
