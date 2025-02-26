# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


class SkiaXCodeApi(recipe_api.RecipeApi):
  XCODE_BUILD_VERSION = '16a242d' # Xcode 16.0

  @property
  def version(self):
    return self.XCODE_BUILD_VERSION

  def install(self):
    # XCode build is listed in parentheses after the version at
    # https://developer.apple.com/news/releases/, or on Wikipedia here:
    # https://en.wikipedia.org/wiki/Xcode#Version_comparison_table
    # Use lowercase letters.
    # https://chrome-infra-packages.appspot.com/p/infra_internal/ios/xcode

    mac_toolchain_cmd = self.m.vars.workdir.joinpath(
        'mac_toolchain', 'mac_toolchain')
    xcode_app_path = self.m.vars.cache_dir.joinpath('Xcode.app')
    # Copied from
    # https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/self.mpy#322
    with self.m.step.nest('ensure xcode') as step_result:
      step_result.step_summary_text = (
          'Ensuring Xcode version %s in %s' % (
              self.version, xcode_app_path))
      install_xcode_cmd = [
          mac_toolchain_cmd, 'install',
          # "ios" is needed for simulator builds
          # (Build-Mac-Clang-x64-Release-iOS).
          '-kind', 'ios',
          '-xcode-version', self.version,
          '-output-dir', xcode_app_path,
      ]
      self.m.step('install xcode', install_xcode_cmd)
      self.m.step('select xcode', [
          'sudo', 'xcode-select', '-switch', xcode_app_path])