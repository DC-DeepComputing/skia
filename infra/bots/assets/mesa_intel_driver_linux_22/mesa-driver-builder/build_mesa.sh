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

if [[ -z "${MESA_VERSION}" ]]; then
    printf "MESA_VERSION environment variable must be provided."
    exit 1
fi

pushd /tmp

#  LIBDRM_VERSION=2.4.113
#  
#  # Install libdrm, because Debian 11.5 doesn't have a recent enough version.
#  curl https://dri.freedesktop.org/libdrm/libdrm-${LIBDRM_VERSION}.tar.xz -o libdrm.tar.xz
#  
#  tar xf libdrm.tar.xz
#  pushd libdrm-${LIBDRM_VERSION}/
#  mkdir -p build/install
#  
#  pushd build
#  
#  meson --buildtype=release     \
#        -Dudev=true             \
#        -Dvalgrind=disabled
#  ninja install
#  
#  popd
#  
#  popd
#  
#  
#  # Install newer wayland-protocols.
#  
#  WAYLAND_PROT_VERSION=1.26
#  
#  curl https://wayland.freedesktop.org/releases/wayland-protocols-1.26.tar.xz -o wayland.tar.xz
#  tar xf wayland.tar.xz
#  
#  pushd wayland-protocols-${WAYLAND_PROT_VERSION}
#  
#  mkdir build
#  
#  pushd build
#  
#  meson --buildtype=release
#  ninja
#  ninja install
#  
#  popd
#  
#  popd


curl  https://archive.mesa3d.org//mesa-${MESA_VERSION}.tar.xz -o mesa.tar.xz
tar xf mesa.tar.xz
cd mesa-${MESA_VERSION}/

ls -al

# https://dri.freedesktop.org/libdrm/libdrm-2.4.113.tar.xz

meson build/
meson configure --prefix="${PWD}/build/install" build/ \
   -D gallium-drivers=swrast,iris \
   -D dri-drivers-path=.
ninja -C build/
ninja -C build/ install

# Copy all the .so files into one a single directory to make things simpler.
find build/install -name "*.so" | xargs -I{} -n1 cp {} /OUT

# Change "library_path" to "./libvulkan_intel.so" and write it out
# in the root of the /OUT directory.
jq <  build/install/share/vulkan/icd.d/intel_icd.x86_64.json '.ICD.library_path="./libvulkan_intel.so"' >  /OUT/intel_icd.x86_64.json

popd
