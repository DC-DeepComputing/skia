#!/usr/bin/env python3
# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys


# Run a command and symbolize anything that looks like a stacktrace in the
# stdout/stderr. This will return with the same error code as the command.
def main(basedir, cmd):
    result = subprocess.run(cmd, encoding="utf-8")
    sys.exit(result.returncode)


if __name__ == '__main__':
    print("symbolize_stack_trace", sys.argv)
    if len(sys.argv) < 3:
        print('USAGE: %s working_dir cmd_and_args...' % sys.argv[0],
              file=sys.stderr)
        sys.exit(1)
    main(sys.argv[1], sys.argv[2:])
