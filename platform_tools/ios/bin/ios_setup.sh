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
IOS_DOCS_DIR="/Documents"

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
  local TARGET="$IOS_MOUNT_POINT/$IOS_DOCS_DIR/$1"

  ios_mount
  rm -rf "$TARGET"
  ios_umount
}

ios_mkdir() {
  local TARGET="$IOS_MOUNT_POINT/$IOS_DOCS_DIR/$1"
  ios_mount
  mkdir -p "$TARGET"
  ios_umount
}

ios_cat() {
  local TARGET="$IOS_DOCS_DIR/$1"
  TMP="$(mktemp)"
  ios_pull "$TARGET" "$TMP"
  cat "$TMP"
  rm "$TMP"
}

# ios_mount: mounts the iOS device for reading or writing.
ios_mount() {
  # If this is already mounted we unmount it.
  if $(mount | grep --quiet "$IOS_MOUNT_POINT"); then
    >&2 echo "Device already mounted at: $IOS_MOUNT_POINT - Unmounting."
    ios_umount || true
  fi

  # Ensure there is a mount directory.
  if [[ ! -d "$IOS_MOUNT_POINT" ]]; then
    mkdir -p $IOS_MOUNT_POINT
  fi
  ifuse --container $IOS_BUNDLE_ID $IOS_MOUNT_POINT

  sleep 2
  if [[ ! -d "${IOS_MOUNT_POINT}/${IOS_DOCS_DIR}" ]]; then
    exit 1
  fi
  >&2 echo "Successfully mounted device."
  #find $IOS_MOUNT_POINT
}

# ios_umount: unmounts the ios device.
ios_umount() {
  sudo umount $IOS_MOUNT_POINT
  sleep 1
}

# ios_restart: restarts the iOS device.
ios_restart() {
  ios_umount || true
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

  idevicefs push "$HOST_SRC" "$IOS_DST"
}

ios_path_exists() {
  local TARGET_PATH="$IOS_DOCS_DIR/$1"
  idevicefs ls "$TARGET_PATH" > /dev/null
  return $?
}
