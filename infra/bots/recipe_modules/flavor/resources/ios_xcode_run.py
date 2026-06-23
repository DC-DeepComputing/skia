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
workdir = os.getcwd()

xcodebuild = os.path.join(
    xcode_path, 'Contents', 'Developer', 'usr', 'bin', 'xcodebuild')

subprocess.check_call([xcodebuild, '-version'])

# PRINT DIAGNOSTICS FOR DIRECTORY LAYOUT AND FILE EXISTENCE
print("=================== SKIA IOS DIAGNOSTICS ===================")
print("app_path: %s" % app_path)
print("app_path exists: %s" % os.path.exists(app_path))
if os.path.exists(app_path):
  print("app_path is dir: %s" % os.path.isdir(app_path))
  if os.path.isdir(app_path):
    try:
      print("app_path contents: %s" % os.listdir(app_path))
    except Exception as e:
      print("failed to list app_path: %s" % e)
app_dir = os.path.dirname(app_path)
print("app_dir: %s" % app_dir)
if os.path.exists(app_dir):
  try:
    print("app_dir contents: %s" % os.listdir(app_dir))
  except Exception as e:
    print("failed to list app_dir: %s" % e)
print("==========================================================")

# Write the .xctestrun file.
# Xcode 15/16 CoreDevice requires the .xctestrun file to reside in the exact same directory
# as the .app / build products directory in order to successfully locate the local build directory.
app_dir = os.path.dirname(app_path)
xctestrun_path = os.path.join(app_dir, 'skia_tests.xctestrun')
app_name = os.path.splitext(os.path.basename(app_path))[0]
module_name = app_name + '_module'

# Create a dummy .xctest bundle inside the build directory to satisfy Xcode 15/16's strict CoreDevice validation.
test_bundle_dir = os.path.join(app_dir, module_name + '.xctest')
os.makedirs(test_bundle_dir, exist_ok=True)

# Copy the compiled binary from the app to the test bundle so it has a valid, physical compiled binary.
src_binary = os.path.join(app_path, app_name)
dst_binary = os.path.join(test_bundle_dir, module_name)
if os.path.exists(src_binary):
  import shutil
  shutil.copy2(src_binary, dst_binary)

# Write the Info.plist for the dummy test bundle.
test_bundle_info_plist = os.path.join(test_bundle_dir, 'Info.plist')
info_plist_contents = {
  'CFBundleIdentifier': bundle_id + '.module',
  'CFBundleName': module_name,
  'CFBundlePackageType': 'BNDL',
  'CFBundleExecutable': module_name,
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
print('xcodebuild exited with code %d' % result)

# Dump the JSON result.
if os.path.exists(output_json_path):
  with open(output_json_path) as f:
    tests = json.load(f)
  print(tests)

# Exit with the code of the app.
exit(result)
