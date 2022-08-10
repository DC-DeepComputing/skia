#!/usr/bin/env python
#
# Copyright 2022 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Package up gcloud's CLI to be stored in CIPD."""


import argparse
import glob
import os
import shutil
import subprocess


# See https://cloud.google.com/sdk/downloads#versioned for documentation on
# scripting gcloud and also for updates.
BASE_URL = 'https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/%s'
GCLOUD_ARCHIVE = 'google-cloud-cli-397.0.0-darwin-arm.tar.gz'
GCLOUD_URL = BASE_URL % GCLOUD_ARCHIVE


def create_asset(target_dir):
  """Download gcloud base CLI and the optional subtools we need."""
  target_dir = os.path.abspath(target_dir)
  subprocess.check_call(['curl', GCLOUD_URL, '-o', GCLOUD_ARCHIVE])

  # Extract the arcive to the target directory and remove it.
  subprocess.check_call(['tar', '-xzf', GCLOUD_ARCHIVE,
                         '--strip-components=1',
                         '-C', target_dir])

  # Substitute the HOME directory in the environment so we don't overwrite
  # an existing gcloud configuration in $HOME/.config/gcloud
  env = os.environ.copy()
  env["HOME"] = target_dir
  gcloud_exe = os.path.join(target_dir, 'bin', 'gcloud')
  subprocess.check_call([gcloud_exe, 'components',
                         'install', 'cloud-datastore-emulator',
                         '--quiet'], env=env)
  subprocess.check_call([gcloud_exe, 'components',
                         'install', 'bigtable',
                         '--quiet'], env=env)
  subprocess.check_call([gcloud_exe, 'components',
                         'install', 'pubsub-emulator',
                         '--quiet'], env=env)
  subprocess.check_call([gcloud_exe, 'components',
                         'install', 'cloud-firestore-emulator',
                         '--quiet'], env=env)
  # As of gcloud v250.0.0 and Cloud Firestore Emulator v1.4.6, there is a bug
  # that something expects the JAR to be executable, but it isn't.
  fs_jar = 'platform/cloud-firestore-emulator/cloud-firestore-emulator.jar'
  subprocess.check_call(['chmod', '+x', os.path.join(target_dir, fs_jar)])
  subprocess.check_call([gcloud_exe, 'components','update', '--quiet'], env=env)

  # Remove the tarball.
  os.remove(GCLOUD_ARCHIVE)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
