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
    # Ignore large resources, nested app resources, and checkout directories to keep logs focused and extremely compact
    if ('Xcode.app' in root or '.git' in root or '/skia/' in root or root.endswith('/skia') or
        '/svg/' in root or root.endswith('/svg') or '/gsutil/' in root or root.endswith('/gsutil') or
        '/skp/' in root or root.endswith('/skp') or '/skimage/' in root or root.endswith('/skimage') or
        '/data/' in root or 'dm.app/data' in root or 'nanobench.app/data' in root or
        '_CodeSignature' in root):
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


def print_post_run_logs(derived_data_dir):
  print("=================== POST-RUN LOGS & DIAGNOSTICS ===================")
  logs_dir = os.path.join(derived_data_dir, 'Logs')
  if not os.path.exists(logs_dir):
    print("Logs directory does not exist: %s" % logs_dir)
    return

  xcresults = []
  for root, dirs, files in os.walk(logs_dir):
    for d in dirs:
      if d.endswith('.xcresult'):
        xcresults.append(os.path.join(root, d))
    for f in files:
      if f.endswith('.log') or f.endswith('.txt') or 'StandardOutput' in f:
        log_path = os.path.join(root, f)
        print("--- Log file: %s ---" % log_path)
        try:
          with open(log_path, 'r', errors='replace') as lf:
            print(lf.read()[:5000]) # Limit to 5000 characters per file to avoid log spam
        except Exception as e:
          print("Failed to read log: %s" % e)

  # Use xcresulttool to dump log actions if xcresults are found
  for xcresult in xcresults:
    print("--- xcresult bundle: %s ---" % xcresult)
    try:
      output = subprocess.check_output([
          'xcrun', 'xcresulttool', 'get', 'object', '--legacy',
          '--path', xcresult,
          '--format', 'json'
      ]).decode('utf-8', errors='replace')
      print("xcresult JSON structure loaded successfully.")
      for r, ds, fs in os.walk(xcresult):
        for f in fs:
          if f.endswith('.log') or f.endswith('.txt') or 'StandardOutput' in f or 'Diagnostics' in f:
            p = os.path.join(r, f)
            print("--- Found xcresult inner file: %s ---" % p)
            with open(p, 'r', errors='replace') as lf:
              print(lf.read()[:5000])
    except Exception as e:
      print("Failed to inspect xcresult with xcresulttool: %s" % e)
  print("================================================================")


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
# We set the dictionary target key to app_name (e.g. 'dm' or 'nanobench') to match the bundle name exactly,
# which completely avoids the "There was a problem finding the local build directory" error under Xcode 16.
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

# Find the ID of the attached device. We just assume a single device is attached.
udid = subprocess.check_output(['idevice_id', '--list']).decode().strip()
destination = 'id=' + udid

output_json_path = os.path.join(workdir, 'enumerate-tests.json')

# Run the app via XCode's privileged test execution tunnel (test-without-building).
# Running within a test session allows Xcode to natively bypass and satisfy provisioning profile trust
# and codesigning checks on physical devices (which are otherwise blocked during raw devicectl launching).
result = subprocess.call([
  xcodebuild, 'test-without-building',
  '-xctestrun', xctestrun_path,
  '-destination', destination,
  '-enumerate-tests',
  '-test-enumeration-format', 'json',
  '-test-enumeration-output-path', output_json_path,
])
print('xcodebuild exited with code %d' % result)

# Print post-run logs and diagnostics to extract application stdout/stderr
try:
  default_derived_data = os.path.expanduser('~/Library/Developer/Xcode/DerivedData')
  print_post_run_logs(default_derived_data)
except Exception as e:
  print("failed to print post-run logs: %s" % e)

# Dump the JSON result.
if os.path.exists(output_json_path):
  with open(output_json_path) as f:
    tests = json.load(f)
  print(tests)

# Exit with the code of the app execution.
exit(result)
