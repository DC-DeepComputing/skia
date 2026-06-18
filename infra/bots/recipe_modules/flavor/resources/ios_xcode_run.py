# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import json
import os
import plistlib
import subprocess
import sys


# Usage: ios_xcode_run.py <path to xcode> <path to app> <bundle id> <args>...
# Runs the given app via xcode with the given args.


xcode_path = sys.argv[1]
app_path = sys.argv[2]
bundle_id = sys.argv[3]
args = sys.argv[4:]

xcodebuild = os.path.join(
    xcode_path, 'Contents', 'Developer', 'usr', 'bin', 'xcodebuild')

subprocess.check_call([xcodebuild, '-version'])

# Write the .xctestrun file.
workdir = os.getcwd()
xctestrun_path = os.path.join(workdir, 'skia_tests.xctestrun')
module_name = os.path.splitext(os.path.basename(app_path))[0] + '_module'

# Create a dummy .xctest bundle inside the .app package to satisfy Xcode 15/16's strict CoreDevice validation.
plugins_dir = os.path.join(app_path, 'PlugIns')
test_bundle_dir = os.path.join(plugins_dir, module_name + '.xctest')
os.makedirs(test_bundle_dir, exist_ok=True)

test_bundle_info_plist = os.path.join(test_bundle_dir, 'Info.plist')
info_plist_contents = {
  'CFBundleIdentifier': bundle_id + '.module',
  'CFBundleName': module_name,
  'CFBundlePackageType': 'BNDL',
  'CFBundleShortVersionString': '1.0',
  'CFBundleVersion': '1',
}
with open(test_bundle_info_plist, 'wb') as f:
  plistlib.dump(info_plist_contents, f)

contents = {
  module_name: {
    'TestBundlePath': test_bundle_dir,
    'TestHostPath': app_path,
    'TestHostBundleIdentifier': bundle_id,
    'TestingEnvironmentVariables': {},
    'EnvironmentVariables': {},
    'CommandLineArguments': args,
  },
}
with open(xctestrun_path, 'wb') as f:
  plistlib.dump(contents, f)

# Find the ID of the attached device. We just assume a single device is attached.
udid = subprocess.check_output(['idevice_id', '--list']).decode().strip()
destination = 'id=' + udid

output_json_path = os.path.join(workdir, 'enumerate-tests.json')

# Run the app via XCode.
result = subprocess.call([
  xcodebuild, 'test-without-building',
  '-xctestrun', xctestrun_path,
  '-destination', destination,
  '-enumerate-tests',
  '-test-enumeration-format', 'json',
  '-test-enumeration-output-path', output_json_path,
])

# Dump the JSON result.
if os.path.exists(output_json_path):
  with open(output_json_path) as f:
    tests = json.load(f)
  print(tests)

# Exit with the code of the app.
exit(result)
