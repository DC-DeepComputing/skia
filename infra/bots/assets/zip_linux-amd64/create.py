#!/usr/bin/env python
#
# Copyright 2022 Google LLC.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create an asset for the linux-amd64 version of the zip executable."""


import argparse
import os
import shutil
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils

# See also https://packages.debian.org/bullseye/amd64/zip/filelist
ZIP_URL = 'https://ftp.debian.org/debian/pool/main/z/zip/zip_3.0-12_amd64.deb'

# See also https://packages.debian.org/bullseye/amd64/libbz2-1.0/filelist
BZIP_URL = 'http://ftp.debian.org/debian/pool/main/b/bzip2/libbz2-1.0_1.0.8-4_amd64.deb'


def download_and_extract_deb(deb_url):
  subprocess.check_call(['wget', deb_url, '--output-document=temp.deb'])
  # The x option for ar means to extract files
  subprocess.check_call(['ar', 'x', 'temp.deb'])  # .deb files are secretly .ar files
  subprocess.check_call(['tar', '--extract', '--file=data.tar.xz'])
  subprocess.check_call(['rm', 'temp.deb'])


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    download_and_extract_deb(ZIP_URL)
    shutil.move('./usr/bin/zip', target_dir)
    # The zip executable is dynamically linked against libbz2.so.1.0, so in case the
    # machine doesn't have that, we make it available. One would need to set
    # LD_LIBRARY_PATH, e.g. https://serverfault.com/a/699902
    download_and_extract_deb(BZIP_URL)
    shutil.move('./lib/x86_64-linux-gnu/libbz2.so.1', target_dir)
    shutil.move('./lib/x86_64-linux-gnu/libbz2.so.1.0', target_dir)
    shutil.move('./lib/x86_64-linux-gnu/libbz2.so.1.0.4', target_dir)
    print(subprocess.check_output(['ls', '-ahl', target_dir], encoding='utf-8'))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

