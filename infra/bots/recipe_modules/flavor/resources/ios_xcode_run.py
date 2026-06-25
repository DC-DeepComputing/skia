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
app_dir = os.path.dirname(app_path)
xctestrun_path = os.path.join(app_dir, 'skia_tests.xctestrun')
app_name = os.path.splitext(os.path.basename(app_path))[0]
module_name = app_name + '_module'

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

# AUTOMATIC CODE RE-SIGNING BLOCK FOR SELF-HEALING PHYSICAL DEVICE RUNS
# Physical iOS devices require the installed app to be signed with a provisioning profile
# that contains the connected device's UDID. Because compile machines are generic, they
# sign the app with generic profiles. On the test bot, we must re-sign the app using the
# bot's own trusted local certificate and provisioning profile.
print("=================== BOT SIGNING AUTO-DISCOVERY ===================")
identity_hash = None
try:
  # Find all valid signing identities on the bot's keychain
  identities_out = subprocess.check_output([
      '/usr/bin/security', 'find-identity', '-v', '-p', 'codesigning'
  ]).decode('utf-8', errors='ignore')
  print("Local signing identities found:\n%s" % identities_out)
  
  # Extract the SHA-1 hash of the first Apple Development / Developer identity
  import re
  m = re.search(r'^\s*\d+\)\s+([A-F0-9]{40})\s+"', identities_out, re.MULTILINE)
  if m:
    identity_hash = m.group(1)
    print("Discovered identity SHA-1: %s" % identity_hash)
except Exception as e:
  print("Warning: failed to query security identities: %s" % e)

profile_path = None
profile_dir = os.path.expanduser('~/Library/MobileDevice/Provisioning Profiles')
if os.path.exists(profile_dir):
  import glob
  profiles = glob.glob(os.path.join(profile_dir, '*.mobileprovision'))
  if profiles:
    profile_path = profiles[0]
    print("Discovered provisioning profile path: %s" % profile_path)

if identity_hash and profile_path:
  print("Re-signing app %s locally with trusted bot credentials..." % app_path)
  try:
    # 1. Copy the provisioning profile to embedded.mobileprovision
    dst_mp = os.path.join(app_path, 'embedded.mobileprovision')
    import shutil
    shutil.copy(profile_path, dst_mp)
    
    # 2. Extract ApplicationIdentifierPrefix from the provisioning profile
    with open(profile_path, 'rb') as f:
      profile_content = f.read().decode('utf-8', errors='ignore')
    m_prefix = re.search(r'<key>ApplicationIdentifierPrefix</key>\s*<array>\s*<string>(.*?)</string>', profile_content)
    prefix = m_prefix.group(1) if m_prefix else "com.google"
    print("Extracted ApplicationIdentifierPrefix: %s" % prefix)
    
    # 3. Create a temporary entitlements plist and sign the app
    import tempfile
    entitlements_str = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
  <dict>
    <key>application-identifier</key>
    <string>{prefix}.com.google.dm</string>
    <key>get-task-allow</key>
    <true/>
  </dict>
</plist>
'''.format(prefix=prefix)
    
    with tempfile.NamedTemporaryFile(suffix='.plist', delete=False) as f:
      f.write(entitlements_str.encode('utf-8'))
      ent_path = f.name
      
    try:
      sign_cmd = [
        'codesign', '--force',
        '--sign', identity_hash,
        '--entitlements', ent_path,
        '--timestamp=none',
        app_path
      ]
      print("Running sign: %s" % ' '.join(sign_cmd))
      subprocess.check_call(sign_cmd)
      print("Codesign completed successfully!")
    finally:
      if os.path.exists(ent_path):
        os.remove(ent_path)
  except Exception as e:
    print("Warning: failed to re-sign app locally: %s" % e)
else:
  print("Warning: Missing identity or provisioning profile. Skipping local re-signing.")
print("==================================================================")

# Find the ID of the attached device. We just assume a single device is attached.
udid = subprocess.check_output(['idevice_id', '--list']).decode().strip()

# Run the app via Apple's native devicectl utility (Xcode 15+ official command line tool for physical iOS 17+ devices).
# This completely bypasses xcodebuild test-without-building hacks (which require XCTest dynamic framework linkage 
# and fail with "local build directory" errors on standalone C++ apps in Xcode 16).
print("Installing app on physical device using devicectl...")
install_cmd = [
  'xcrun', 'devicectl', 'device', 'install', 'app',
  '--device', udid,
  app_path
]
print("Running: %s" % ' '.join(install_cmd))
subprocess.check_call(install_cmd)

print("Launching app on physical device using devicectl...")
launch_cmd = [
  'xcrun', 'devicectl', 'device', 'process', 'launch',
  '--device', udid,
  bundle_id,
  '--'
] + args
print("Running: %s" % ' '.join(launch_cmd))
result = subprocess.call(launch_cmd)
print("devicectl launch exited with code %d" % result)

# Exit with the code of the app execution.
exit(result)
