#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script has GN shell out to Bazel in order to compile all the skslc test files

import subprocess
import sys
import platform

sksl_tests = ['glsl_tests', 'glsl_nosettings_tests', 'metal_tests', 'skvm_tests',
              'stage_tests', 'spirv_tests', 'wgsl_tests']

for target in sksl_tests:
    cmd = ['bazelisk', 'run', '//tools/skslc:compile_'+target]
    if platform.system() == 'Darwin':
        cmd += ["--spawn_strategy=local"]
    result = subprocess.run(cmd, encoding='utf-8',stdout=sys.stderr)
    if result.returncode != 0:
        sys.exit(result.returncode)
