#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

import os
import sys
import subprocess

subprocess.run(["cmake", "--version"], check=True)
subprocess.run(["ninja", "--version"], check=True)

print("kjlubick", os.getcwd())

configure_cmd = [
"cmake",
"-S", "../../third_party/externals/dawn", # source
"-B", "cmake/dawn",
"-DDAWN_FETCH_DEPENDENCIES=ON", "-DDAWN_ENABLE_INSTALL=ON", "-DCMAKE_BUILD_TYPE=Release", "-DDAWN_USE_X11=OFF", "-G", "Ninja",
]

subprocess.run(configure_cmd, check=True)