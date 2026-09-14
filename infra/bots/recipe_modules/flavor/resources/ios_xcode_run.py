# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import subprocess
import sys


# Usage: ios_xcode_run.py <path to xcode> <path to app> <bundle id> <args>...
# Runs the given app via xcode with the given args.


xcode_path = sys.argv[1]
app_path = sys.argv[2]
bundle_id = sys.argv[3]
args = sys.argv[4:]

# Ensure xcrun uses the requested Xcode version.
os.environ['DEVELOPER_DIR'] = xcode_path

xcodebuild = os.path.join(
    xcode_path, 'Contents', 'Developer', 'usr', 'bin', 'xcodebuild')

subprocess.check_call([xcodebuild, '-version'])

# Find the ID of the attached device. We just assume a single device is attached.
udid = subprocess.check_output(['idevice_id', '--list']).decode().strip().splitlines()[0]

# Install the app on the device via devicectl.
install_cmd = [
    'xcrun', 'devicectl', 'device', 'install', 'app',
    '--device', udid,
    app_path,
]
print('Installing app: %s' % ' '.join(install_cmd))
try:
  subprocess.check_call(install_cmd)
except subprocess.CalledProcessError as e:
  print('Failed to install app on device: %s' % e, file=sys.stderr)
  sys.exit(e.returncode)

# Launch the app and stream its console output until exit.
launch_cmd = [
    'xcrun', 'devicectl', 'device', 'process', 'launch',
    '--device', udid,
    '--terminate-existing',
    '--console',
    bundle_id,
    '--',
] + args
print('Launching app: %s' % ' '.join(launch_cmd))
result = subprocess.call(launch_cmd)
print('devicectl launch exited with code %d' % result)

sys.exit(result)
