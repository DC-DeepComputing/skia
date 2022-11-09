#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script has GN shell out to Bazel in order to minify the test sources.

import subprocess
import sys
import platform

cmd = ['bazelisk', 'run', '//tools/sksl-minify:minify_tests']
if platform.system() == 'Darwin':
    cmd += ["--spawn_strategy=local"]
result = subprocess.run(cmd, encoding='utf-8')
sys.exit(result.returncode)

