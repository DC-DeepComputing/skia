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
    
    info_plist_path = os.path.join(app_path, 'Info.plist')
    if os.path.exists(info_plist_path):
      try:
        with open(info_plist_path, 'rb') as f:
          plist_data = plistlib.load(f)
        print("dm.app/Info.plist contents: %s" % plist_data)
      except Exception as e:
        print("failed to load dm.app/Info.plist: %s" % e)

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
# In addition, we map it to the actual app name to prevent any mismatch issues.
app_dir = os.path.dirname(app_path)
xctestrun_path = os.path.join(app_dir, 'skia_tests.xctestrun')
app_name = os.path.splitext(os.path.basename(app_path))[0]
contents = {
  app_name: {
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

# Create a standard Xcode DerivedData structure to satisfy Xcode 15/16's local build directory lookup.
derived_data_dir = os.path.join(workdir, 'DerivedData')
products_dir = os.path.join(derived_data_dir, 'Build', 'Products')

# Since our task can be Release or Debug, we can create both config folders to be 100% safe and robust.
active_xctestrun_path = None

for config in ['Release-iphoneos', 'Debug-iphoneos']:
  target_dir = os.path.join(products_dir, config)
  os.makedirs(target_dir, exist_ok=True)
  
  # Symlink or copy dm.app into the build products config directory
  dst_app = os.path.join(target_dir, os.path.basename(app_path))
  if not os.path.exists(dst_app):
    import shutil
    try:
      os.symlink(app_path, dst_app)
    except Exception as e:
      print("Failed to symlink %s to %s: %s. Copying instead." % (app_path, dst_app, e))
      shutil.copytree(app_path, dst_app)
      
  # Write the .xctestrun file inside the config folder
  target_xctestrun_path = os.path.join(target_dir, 'skia_tests.xctestrun')
  target_contents = {
    app_name: {
      'TestBundlePath': dst_app,
      'TestHostPath': dst_app,
      'TestHostBundleIdentifier': bundle_id,
      'TestingEnvironmentVariables': {},
      'EnvironmentVariables': {},
      'CommandLineArguments': args,
    },
  }
  with open(target_xctestrun_path, 'wb') as f:
    plistlib.dump(target_contents, f)
    
  if config.split('-')[0].lower() in app_path.lower():
    active_xctestrun_path = target_xctestrun_path

if not active_xctestrun_path:
  active_xctestrun_path = os.path.join(products_dir, 'Release-iphoneos', 'skia_tests.xctestrun')

# Find the ID of the attached device. We just assume a single device is attached.
udid = subprocess.check_output(['idevice_id', '--list']).decode().strip()
destination = 'id=' + udid

output_json_path = os.path.join(workdir, 'enumerate-tests.json')

# Run the app via XCode.
result = subprocess.call([
  xcodebuild, 'test-without-building',
  '-xctestrun', active_xctestrun_path,
  '-destination', destination,
  '-derivedDataPath', derived_data_dir,
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
