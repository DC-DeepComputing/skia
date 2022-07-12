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
meson configure --prefix="${PWD}/build/install" build/
ninja -C build/
ninja -C build/ install
rsync -av build/install/lib/x86_64-linux-gnu/ /OUT

popd
