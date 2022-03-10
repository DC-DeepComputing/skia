#!/bin/bash
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
if [[ "$@" == *SKIA_SKIP_IWYU* || "$@" == *use-ld* ]]
then
  external/clang_linux_amd64/bin/clang $@
else
  # We want to fail before IWYU if clang fails
  set -e
  external/clang_linux_amd64/bin/clang $@
  # IWYU always returns a non-zero code because it doesn't produce the .o file (that's why
  # we ran Clang first).
  set +e
  # Get absolute path to the mapping file because resolving the relative path is tricky, given
  # how Bazel locates the toolchain files.
  MAPPING_FILE=$(realpath $(dirname ${BASH_SOURCE[0]}))"/IWYU_mapping.imp"
  # IWYU always outputs something to stderr, which can be noisy if everything is fixed.
  external/clang_linux_amd64/usr/bin/include-what-you-use \
      -Xiwyu --mapping_file=$MAPPING_FILE $@ 2>/dev/null
  # IWYU returns 2 if everything looks good.
  if [ $? -eq 2 ];then
    exit 0
  else
    # Run IWYU again, but this time display the output. Then return non-zero to fail the build.
    external/clang_linux_amd64/usr/bin/include-what-you-use \
        -Xiwyu --mapping_file=$MAPPING_FILE -Xiwyu --no_comments \
        -Xiwyu --quoted_includes_first -Xiwyu --verbose=3 $@
    exit 1
  fi
fi

