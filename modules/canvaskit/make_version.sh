#!/bin/bash -e

# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Write the SKIA_VERSION to a JavaScript file.

if [ "$1" == "" ]
then
  echo "Must supply output version.js file path." >&2
  exit 1
fi

SCRIPT_DIR=$(dirname $(realpath $0))
WORKSPACE_DIR=$(realpath "${SCRIPT_DIR}/../..")
OUT_DIR="${WORKSPACE_DIR}/bazel-out"
STABLE_STATUS="${OUT_DIR}/stable-status.txt"
VERSION_JS_PATH=$1
GIT_REVISION=$(grep STABLE_GIT_REVISION ${STABLE_STATUS} | awk '{print $2}' )

mkdir -p $(dirname ${VERSION_JS_PATH})
echo "const SKIA_VERSION = '${GIT_REVISION}';" > ${VERSION_JS_PATH}
