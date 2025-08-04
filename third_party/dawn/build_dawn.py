#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

import os
import sys
import shutil
import subprocess
import hashlib

outputs = {}
for arg in sys.argv[1:]:
    if "=" in arg:
        # "../../bazel-bin/src/ports/ffi.h=src/core/ffi.h" means to
        # copy ../../bazel-bin/src/ports/ffi.h to
        # $GN_OUTPUT/src/core/ffi.h
        cmake_file, output_path = arg.split("=")
        outputs[cmake_file] = output_path
    else:
        # "../../bazel-bin/src/ports/libstuff.a" means to copy
        # ../../bazel-bin/src/ports/libstuff.a to
        # $GN_OUTPUT/libstuff.a
        outputs[arg] = os.path.basename(arg)

subprocess.run(["cmake", "--version"], check=True)
subprocess.run(["ninja", "--version"], check=True)

print("kjlubick", os.getcwd())

configure_cmd = [
"cmake",
"-S", "../../third_party/externals/dawn", # source
"-B", "cmake/dawn",
"-DDAWN_FETCH_DEPENDENCIES=ON",
"-DDAWN_ENABLE_INSTALL=ON",
"-DCMAKE_BUILD_TYPE=Release",
"-DDAWN_USE_X11=OFF",
"-DTINT_BUILD_WGSL_READER=ON",
"-DTINT_BUILD_WGSL_WRITER=ON",
"-DTINT_ENABLE_INSTALL=ON",
"-G", "Ninja",
]

subprocess.run(configure_cmd, check=True)

configure_cmd = [
"cmake",
"--build", "cmake/dawn",
]

subprocess.run(configure_cmd, check=True)

for cmake_file, output_path in outputs.items():
    # GN expects files to be created underneath the output directory from which
    # this script is invoked.
    expected_output = os.path.join(os.getcwd(), output_path)
    if not os.path.exists(expected_output):
        shutil.copyfile(cmake_file, expected_output)
        os.chmod(expected_output, 0o755)
    else:
        # GN uses timestamps to determine if it should re-build a file. If the
        # two files match (via hash) we should not re-copy the output, which would
        # re-trigger subsequent rebuilds.
        created_hash = hashlib.sha256(open(cmake_file, 'rb').read()).hexdigest()
        existing_hash = hashlib.sha256(open(expected_output, 'rb').read()).hexdigest()
        if created_hash != existing_hash:
            os.remove(expected_output)
            shutil.copyfile(cmake_file, expected_output)
            os.chmod(expected_output, 0o755)
