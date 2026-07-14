#!/usr/bin/env python3
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the mesa driver."""


import argparse
import os
import subprocess

MESA_VERSION = '24.2.0'

def create_asset(target_dir):
  """Create the asset."""
  my_dir = os.path.abspath(os.path.dirname(__file__))
  builder_dir = os.path.join(my_dir, 'mesa-driver-builder')
  
  cmd = [
    'docker', 'build', '-t', 'mesa-driver-builder-24:latest',
    builder_dir,
  ]
  print('Building container', cmd)
  subprocess.check_output(cmd)

  cmd = [
    'docker', 'run', '--volume', '%s:/OUT' % target_dir,
    '--env', 'MESA_VERSION=%s' % MESA_VERSION,
    'mesa-driver-builder-24:latest'
  ]
  print('Running container', cmd)
  subprocess.check_output(cmd)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
