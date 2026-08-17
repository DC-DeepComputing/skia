#!/usr/bin/env python
#
# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys
import os
import shlex

def main():
    if len(sys.argv) != 3:
        print("false")
        return

    compiler = sys.argv[1]
    flag = sys.argv[2]

    devnull = 'NUL' if os.name == 'nt' else '/dev/null'

    # We use -Werror to ensure that if the compiler emits a warning for an
    # unknown flag, it fails the check.
    compiler_cmd = shlex.split(compiler)
    cmd = compiler_cmd + ['-Werror', flag, '-c', '-x', 'c', devnull, '-o', devnull]

    try:
        with open(os.devnull, 'w') as devnull_file:
            subprocess.check_call(cmd, stdout=devnull_file, stderr=devnull_file)
        print("true")
    except subprocess.CalledProcessError:
        print("false")
    except OSError:
        print("false")

if __name__ == '__main__':
    main()
