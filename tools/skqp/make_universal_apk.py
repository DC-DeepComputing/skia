#! /usr/bin/env python
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
This script can be run with no arguments, in which case it will produce an
APK with native libraries for all four architectures: arm, arm64, x86, and
x64.  You can instead list the architectures you want as arguments to this
script.  For example:

    python make_universal_apk.py arm x86

The environment variables ANDROID_NDK_HOME and ANDROID_HOME must be set to the
locations of the Android NDK and SDK.

Additionally, `ninja` should be in your path.

It assumes that the source tree is in the desired state, e.g. by having
run 'python tools/git-sync-deps' in the root of the skia checkout.

Also:
  * If the environment variable SKQP_BUILD_DIR is set, many of the
    intermediate build objects will be placed here.
  * If the environment variable SKQP_OUTPUT_DIR is set, the final APK
    will be placed in this directory.
  * If the environment variable SKQP_DEBUG is set, Skia will be compiled
    in debug mode.
'''

import os
import sys

import create_apk

def make_apk(opts):
    assert '/' in [os.sep, os.altsep]

    create_apk.makedirs(opts.build_dir)

    # Create symlinks to each top-level item in resources. The symlinks will be
    # in the assets folder, so that the app can access them directly.
    skia_dir = os.path.abspath(os.path.normpath(
        os.path.join(os.path.dirname(__file__), '../..')))
    assets_dir = os.path.join(skia_dir,
        'platform_tools/android/apps/skqp/src/main/assets/')
    skia_resources_dir = os.path.join(skia_dir, 'resources')
    symlinks = []
    for resource in os.listdir(skia_resources_dir):
        symlinked_resource_path = os.path.join(assets_dir, resource)
        if os.path.exists(symlinked_resource_path):
            # symlink may be out of date, so remove and update.
            os.remove(symlinked_resource_path)
        os.symlink(os.path.join(skia_resources_dir, resource),
            symlinked_resource_path)
        symlinks.append(symlinked_resource_path)

    create_apk.create_apk(opts)

    for symlink in symlinks:
        os.remove(symlink)

def main():
    options = create_apk.SkQP_Build_Options()
    if options.error:
        sys.stderr.write(options.error + __doc__)
        sys.exit(1)
    options.write(sys.stdout)
    make_apk(options)

if __name__ == '__main__':
    main()
