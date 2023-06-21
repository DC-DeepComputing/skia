#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import subprocess


VERSION = "1.3.230602002"
SHORT_VERSION = "1.3"
URL = "https://www.nuget.org/api/v2/package/Microsoft.WindowsAppSDK/%s"


def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(["mkdir", "%s/tmp" % target_dir])
  subprocess.check_call(["curl", "-L", URL % VERSION, "-o", "%s/tmp/windowsappsdk.zip" %target_dir])
  subprocess.check_call(["unzip", "%s/tmp/windowsappsdk.zip" % target_dir, "-d", "%s/tmp/sdk" % target_dir])
  subprocess.check_call(["unzip", "%s/tmp/sdk/tools/MSIX/win10-x64/Microsoft.WindowsAppRuntime.%s.msix" % (target_dir, SHORT_VERSION), "-d", "%s/tmp/runtime" % target_dir])

  subprocess.check_call(["mkdir", "%s/include" % target_dir])
  subprocess.check_call(["mkdir", "%s/bin" % target_dir])
  subprocess.check_call(["cp", "%s/tmp/sdk/include/dwrite.h" % target_dir, "%s/include" % target_dir])
  subprocess.check_call(["cp", "%s/tmp/sdk/include/dwrite_1.h" % target_dir, "%s/include" % target_dir])
  subprocess.check_call(["cp", "%s/tmp/sdk/include/dwrite_2.h" % target_dir, "%s/include" % target_dir])
  subprocess.check_call(["cp", "%s/tmp/sdk/include/dwrite_3.h" % target_dir, "%s/include" % target_dir])
  subprocess.check_call(["cp", "%s/tmp/sdk/include/dwrite_core.h" % target_dir, "%s/include" % target_dir])
  subprocess.check_call(["cp", "%s/tmp/runtime/DWriteCore.dll" % target_dir, "%s/bin" % target_dir])

  subprocess.check_call(["rm", "-rf", "%s/tmp" % target_dir])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
