# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import json
import os
import plistlib
import subprocess
import sys


def write_xctestrun(xctestrun_path, app_path, bundle_id, args):
    module_name = os.path.splitext(os.path.basename(app_path))[0] + '_module'
    contents = {
        module_name: {
            'TestBundlePath': app_path,
            'TestHostPath': app_path,
            'TestHostBundleIdentifier': bundle_id,
            'TestingEnvironmentVariables': {},
            'EnvironmentVariables': {},
            'CommandLineArguments': args,
        },
    }
    with open(xctestrun_path, 'wb') as f:
        plistlib.dump(contents, f)


xcode_path = sys.argv[1]
app_path = sys.argv[2]
bundle_id = sys.argv[3]
args = sys.argv[4:]

xcodebuild = os.path.join(
    xcode_path, 'Contents', 'Developer', 'usr', 'bin', 'xcodebuild')

subprocess.check_call([xcodebuild, '-version'])

workdir = os.getcwd()
xctestrun_path = os.path.join(workdir, 'skia_tests.xctestrun')
write_xctestrun(xctestrun_path, app_path, bundle_id, args)

# TODO(borenet): This seems to be a pointer to an emulator; is there an analog
# to a physical device?
destination = 'id=cf9ad1ea38a7449e203d22eaddc90d2164206817'
output_json_path = os.path.join(workdir, 'enumerate-tests.json')

result = subprocess.call([
    xcodebuild, 'test-without-building',
    '-xctestrun', xctestrun_path,
    #'-destination', destination,
    '-enumerate-tests',
    '-test-enumeration-format', 'json',
    '-test-enumeration-output-path', output_json_path,
])

if os.path.exists(output_json_path):
    with open(output_json_path) as f:
        tests = json.load(f)
    print(tests)

exit(result)