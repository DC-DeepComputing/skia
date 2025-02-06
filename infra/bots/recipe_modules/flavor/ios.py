# Copyright 2025 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import default


"""iOS flavor, used for running code on iOS."""


class iOSFlavor(default.DefaultFlavor):
  def __init__(self, m, app_name):
    super(iOSFlavor, self).__init__(m, app_name)
    self.device_dirs = default.DeviceDirs(
        bin_dir='[unused]',
        dm_dir='dm',
        perf_data_dir='perf',
        resource_dir='resources',
        fonts_dir = 'NOT_SUPPORTED',
        images_dir='images',
        lotties_dir='lotties',
        skp_dir='skps',
        svg_dir='svgs',
        tmp_dir='tmp',
        texttraces_dir='')

  @property
  def env(self):
    return {
      #'IOS_BUNDLE_ID': 'com.google.%s' % self.app_name,
      #'IOS_MOUNT_POINT': self.m.vars.workdir.joinpath('mnt_iosdevice'),
    }

  def context(self):
    return self.m.context(env=self.env)

  def install(self):
    # XCode build is listed in parentheses after the version at
    # https://developer.apple.com/news/releases/, or on Wikipedia here:
    # https://en.wikipedia.org/wiki/Xcode#Version_comparison_table
    # Use lowercase letters.
    # https://chrome-infra-packages.appspot.com/p/infra_internal/ios/xcode
    XCODE_BUILD_VERSION = '16c5032a' # Xcode 16.2
    mac_toolchain_cmd = self.m.vars.workdir.joinpath(
        'mac_toolchain', 'mac_toolchain')
    xcode_app_path = self.m.vars.cache_dir.joinpath('Xcode.app')
    # Copied from
    # https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/api.py#322
    with self.m.step.nest('ensure xcode') as step_result:
      step_result.step_summary_text = (
          'Ensuring Xcode version %s in %s' % (
              XCODE_BUILD_VERSION, xcode_app_path))
      install_xcode_cmd = [
          mac_toolchain_cmd, 'install',
          # "ios" is needed for simulator builds
          # (Build-Mac-Clang-x64-Release-iOS).
          '-kind', 'ios',
          '-xcode-version', XCODE_BUILD_VERSION,
          '-output-dir', xcode_app_path,
      ]
      self.m.step('install xcode', install_xcode_cmd)
      self.m.step('select xcode', [
          'sudo', 'xcode-select', '-switch', xcode_app_path])

    # Install app (necessary before copying any resources to the device).
    if self.app_name:
      app_package = self.host_dirs.bin_dir.joinpath('%s.app' % self.app_name)

      def uninstall_app(attempt):
        # If app ID changes, upgrade will fail, so try uninstalling.
        self.m.run(self.m.step,
                   'uninstall %s' % self.app_name,
                   cmd=['ideviceinstaller', '-U',
                        'com.google.%s' % self.app_name],
                   infra_step=True,
                   # App may not be installed.
                   abort_on_failure=False, fail_build_on_failure=False)

      num_attempts = 2
      self.m.run.with_retry(self.m.step, 'install %s' % self.app_name,
                            num_attempts,
                            cmd=['ideviceinstaller', '-i', app_package],
                            between_attempts_fn=uninstall_app,
                            infra_step=True)

  def step(self, name, cmd, **kwargs):
    app_name = cmd[0]
    bundle_id = 'com.google.%s' % app_name
    args = ['--start', bundle_id, '--args'] + [str(ele) for ele in cmd[1:]]
    success = False
    with self.context():
      try:
        self.m.run(self.m.step, name, cmd=['idevice-app-runner'] + args)
        success = True
      finally:
        if not success:
          self.m.run(
              self.m.step, '%s with full debug output' % name,
              cmd=['python3', self.module.resource('ios_debug_cmd.py')] + args)

  def _run_ios_script(self, script, first, *rest):
    with self.context():
      full = self.m.path.start_dir.joinpath(
          'skia', 'platform_tools', 'ios', 'bin', 'ios_' + script)
      self.m.run(self.m.step,
                 name = '%s %s' % (script, first),
                 cmd = [full, first] + list(rest),
                 infra_step=True)

  def copy_file_to_device(self, host, device):
    self._run_ios_script('push_file', host, device)

  def copy_directory_contents_to_device(self, host, device):
    self._run_ios_script('push_if_needed', host, device)

  def copy_directory_contents_to_host(self, device, host):
    self._run_ios_script('pull_if_needed', device, host)

  def remove_file_on_device(self, path):
    self._run_ios_script('rm', path)

  def create_clean_device_dir(self, path):
    self._run_ios_script('rm',    path)
    self._run_ios_script('mkdir', path)

  def read_file_on_device(self, path, **kwargs):
    with self.context():
      full = self.m.path.start_dir.joinpath(
          'skia', 'platform_tools', 'ios', 'bin', 'ios_cat_file')
      rv = self.m.run(self.m.step,
                      name = 'cat_file %s' % path,
                      cmd = [full, path],
                      stdout=self.m.raw_io.output(),
                      infra_step=True,
                      **kwargs)
      return rv.stdout.decode('utf-8').rstrip() if rv and rv.stdout else None
