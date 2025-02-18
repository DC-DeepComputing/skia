#!/bin/bash
###############################################################################
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
###############################################################################
#
# ios_setup.sh: Sets environment variables used by other iOS scripts.

# File system location where we mount the ios devices.
if [[ -z "${IOS_MOUNT_POINT}" ]]; then
  IOS_MOUNT_POINT="/tmp/mnt_iosdevice"
fi

# Location on the ios device where all data are stored. This is
# relative to the mount point.
IOS_DOCS_DIR="Documents"

# Directory with the Skia source.
SKIA_SRC_DIR=$(cd "${SCRIPT_DIR}/../../.."; pwd)

# Provisioning profile - this needs to be set up on the local machine.
PROVISIONING_PROFILE=""

# Code Signing identity - this needs to be set up on the local machine.
CODE_SIGN_IDENTITY="iPhone Developer"

IOS_RESULTS_DIR="results"

# Location of XCode build products.
if [[ -z "$XCODEBUILD" ]]; then
  XCODEBUILD="${SKIA_SRC_DIR}/xcodebuild"
fi

ios_rm() {
  local TARGET="$IOS_DOCS_DIR/$1"
  idevicefs rm -Rfd "$TARGET"
}

ios_mkdir() {
  local TARGET="$IOS_DOCS_DIR/$1"
  cpy="$(mktemp -d)"
  idevicefs push "$cpy" "$TARGET"
  rm -rf "$cpy"
}

ios_cat() {
  ios_pull "$1" /dev/stdout
}

# ios_restart: restarts the iOS device.
ios_restart() {
  idevicediagnostics restart
}

# ios_pull(ios_src, host_dst): Copies the content of ios_src to host_dst.
# The path is relative to the 'Documents' folder on the device.
ios_pull() {
  # read input params
  local IOS_SRC="$IOS_DOCS_DIR/$1"
  local HOST_DST="$2"

  idevicefs pull "$IOS_SRC" "$HOST_DST"
}

# ios_push(host_src, ios_dst)
ios_push() {
  # read input params
  local HOST_SRC="$1"
  local IOS_DST="$IOS_DOCS_DIR/$2"

  dir="$(dirname $2)"
  ios_path_exists "$dir" || ios_mkdir "$dir"
  idevicefs push "$HOST_SRC" "$IOS_DST"
}

ios_path_exists() {
  local TARGET_PATH="$IOS_DOCS_DIR/$1"
  idevicefs ls "$TARGET_PATH" > /dev/null
  return $?
}

#   ANDROID_LS=`$ADB $DEVICE_SERIAL shell ls -ld $ANDROID_DST`
#   HOST_LS=`ls -ld $HOST_SRC`
#   if [ "${ANDROID_LS:0:1}" == "d" -a "${HOST_LS:0:1}" == "-" ];
#   then
#     ANDROID_DST="${ANDROID_DST}/$(basename ${HOST_SRC})"
#   fi


#   ANDROID_LS=`$ADB $DEVICE_SERIAL shell ls -ld $ANDROID_DST`
#   if [ "${ANDROID_LS:0:1}" == "-" ]; then
#     #get the MD5 for dst and src
#     ANDROID_MD5=`$ADB $DEVICE_SERIAL shell md5 $ANDROID_DST`
#     if [ $(uname) == "Darwin" ]; then
#       HOST_MD5=`md5 -q $HOST_SRC`
#     else
#       HOST_MD5=`md5sum $HOST_SRC`
#     fi

#     if [ "${ANDROID_MD5:0:32}" != "${HOST_MD5:0:32}" ]; then
#       echo -n "$ANDROID_DST "
#       $ADB $DEVICE_SERIAL push $HOST_SRC $ANDROID_DST
#     fi
#   elif [ "${ANDROID_LS:0:1}" == "d" ]; then
#     for FILE_ITEM in `ls $HOST_SRC`; do
#       adb_push_if_needed "${HOST_SRC}/${FILE_ITEM}" "${ANDROID_DST}/${FILE_ITEM}"
#     done
#   else
#     HOST_LS=`ls -ld $HOST_SRC`
#     if [ "${HOST_LS:0:1}" == "d" ]; then
#       $ADB $DEVICE_SERIAL shell mkdir -p $ANDROID_DST
#       adb_push_if_needed $HOST_SRC $ANDROID_DST
#     else
#       echo -n "$ANDROID_DST "
#       $ADB $DEVICE_SERIAL shell mkdir -p "$(dirname "$ANDROID_DST")"
#       $ADB $DEVICE_SERIAL push $HOST_SRC $ANDROID_DST
#     fi
#   fi
# }

# setup_device "${DEVICE_ID}"
