#!/bin/bash
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This assumes it is being run inside a docker container with the following
# mounts:
#  /SRC: Swarming start dir
#  /OUT: output directory for gn and ninja

set -e
set -x

apt-get update
apt-get install -y wget unzip lld git

mkdir /tmp/bazelisk_dl
pushd /tmp/bazelisk_dl
wget -O bazelisk.zip https://chrome-infra-packages.appspot.com/dl/skia/bots/bazelisk_linux_amd64/+/version:0
unzip bazelisk.zip
popd

export PATH="/tmp/bazelisk_dl:${PATH}"

gcc --version
cd /SRC/$1
bazelisk build $2