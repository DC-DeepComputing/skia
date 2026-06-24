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


def print_directory_tree(start_dir):
  print("=================== COMPLETE DIRECTORY TREE ===================")
  print("Starting from: %s" % start_dir)
  for root, dirs, files in os.walk(start_dir):
    # Ignore large resources and directories to keep logs focused, clean, and extremely compact
    if ('Xcode.app' in root or '.git' in root or '/skia/' in root or root.endswith('/skia') or
        '/svg/' in root or root.endswith('/svg') or '/gsutil/' in root or root.endswith('/gsutil') or
        '/skp/' in root or root.endswith('/skp') or '/skimage/' in root or root.endswith('/skimage')):
      continue
    print("Tree: %s" % root)
    for d in dirs:
      d_path = os.path.join(root, d)
      is_sym = os.path.islink(d_path)
      try:
        target = os.readlink(d_path) if is_sym else ""
      except Exception:
        target = "<failed to read link>"
      print("  [DIR] %s (Symlink: %s, Target: %s)" % (d, is_sym, target))
    for f in files:
      f_path = os.path.join(root, f)
      is_sym = os.path.islink(f_path)
      try:
        target = os.readlink(f_path) if is_sym else ""
      except Exception:
        target = "<failed to read link>"
      try:
        size = os.path.getsize(f_path) if not is_sym else 0
      except Exception:
        size = -1
      print("  [FILE] %s (Size: %s, Symlink: %s, Target: %s)" % (f, size, is_sym, target))
  print("==============================================================")


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
app_dir = os.path.dirname(app_path)
xctestrun_path = os.path.join(app_dir, 'skia_tests.xctestrun')
app_name = os.path.splitext(os.path.basename(app_path))[0]
module_name = app_name + '_module'

# Create a standard Xcode DerivedData structure to satisfy Xcode 15/16's local build directory lookup.
derived_data_dir = os.path.join(workdir, 'DerivedData')
products_dir = os.path.join(derived_data_dir, 'Build', 'Products')

# Since our task can be Release or Debug, we can create both config folders to be 100% safe and robust.
active_xctestrun_path = None

for config in ['Release-iphoneos', 'Debug-iphoneos']:
  target_dir = os.path.join(products_dir, config)
  os.makedirs(target_dir, exist_ok=True)
  
  # 1. Copy dm.app into the build products config directory
  # We must physically copy (not symlink) dm.app so that its real path
  # resides inside DerivedData, which is required for Xcode 15/16 CoreDevice validation.
  dst_app = os.path.join(target_dir, os.path.basename(app_path))
  if not os.path.exists(dst_app):
    import shutil
    shutil.copytree(app_path, dst_app)
      
  # 2. Create a dummy .xctest bundle inside the build directory to satisfy Xcode 15/16's strict CoreDevice validation.
  test_bundle_dir = os.path.join(target_dir, module_name + '.xctest')
  os.makedirs(test_bundle_dir, exist_ok=True)

  # Copy the compiled binary from the app to the test bundle so it has a valid, physical compiled binary.
  src_binary = os.path.join(app_path, app_name)
  dst_binary = os.path.join(test_bundle_dir, module_name)
  if os.path.exists(src_binary) and not os.path.exists(dst_binary):
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
      
  # 3. Write the .xctestrun file inside the config folder
  target_xctestrun_path = os.path.join(target_dir, 'skia_tests.xctestrun')
  target_contents = {
    module_name: {
      'TestBundlePath': test_bundle_dir,
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

# Print entire recursive directory tree starting from swarming task root
try:
  print_directory_tree(os.path.dirname(workdir))
except Exception as e:
  print("failed to print directory tree: %s" % e)

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
