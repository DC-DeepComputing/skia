#!/usr/bin/env python3
#
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess
import tempfile
import shutil


def create_asset(target_dir):
  """Copy the fonts from two different git repos into one folder."""
  os.makedirs(target_dir, exist_ok=True)
  with tempfile.TemporaryDirectory() as tmp:
    os.chdir(tmp)
    subprocess.call(['git', 'clone', 'https://github.com/Rusino/textlayout'])
    subprocess.call(['git', 'clone', 'https://skia.googlesource.com/skia/'])

    os.chdir(os.path.join(tmp, "textlayout"))
    subprocess.call(['git', 'checkout', '9c1868e84da1db358807ebff5cf52327e53560a0'])
    shutil.copytree("fonts", target_dir, dirs_exist_ok=True)

    os.chdir(os.path.join(tmp, "skia"))
    subprocess.call(['git', 'checkout', '2f82ef6e77774dc4e8e382b2fb6159c58c0f8725'])
    shutil.copytree(os.path.join("resources", "fonts"), target_dir, dirs_exist_ok=True)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

