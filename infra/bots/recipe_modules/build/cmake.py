# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from . import util


def finalize_gn_flags(args):
  if args.get('extra_cflags'):
    args['extra_cflags'] = repr(args['extra_cflags']).replace("'", '"')
  if args.get('extra_ldflags'):
    args['extra_ldflags'] = repr(args['extra_ldflags']).replace("'", '"')
  return ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.items()))


def compile_fn(api, checkout_root, _ignore):
  out_dir = api.vars.cache_dir.joinpath('docker', 'cmake')
  configuration = api.vars.builder_cfg.get('configuration', '')
  if configuration != 'Debug':
    args['is_debug'] = 'false'

  skia_dir      = checkout_root.joinpath('skia')
  extra_tokens  = api.vars.extra_tokens

  with api.context(cwd=skia_dir):
    api.run(api.step, 'fetch-gn',
            cmd=['python3', skia_dir.joinpath('bin', 'fetch-gn')],
            infra_step=True)

    api.run(api.step, 'fetch-ninja',
            cmd=['python3', skia_dir.joinpath('bin', 'fetch-ninja')],
            infra_step=True)

  workdir = api.path.start_dir
  args, env, ccache = get_compile_flags(api, checkout_root, out_dir, workdir)
  gn_args = finalize_gn_flags(args)
  gn = skia_dir.joinpath('bin', 'gn')
  ninja = skia_dir.joinpath('third_party', 'ninja', 'ninja')

  with api.context(cwd=skia_dir):
    with api.env(env):
      if ccache:
        api.run(api.step, 'ccache stats-start', cmd=[ccache, '-s'])
      api.run(api.step, 'gn gen',
              cmd=[gn, 'gen', out_dir, '--args=' + gn_args,
                   '--ide=json',
                   '--json-ide-script=' + out_dir + '/gn/gn_to_cmake.py',
                   '--json-ide-script-args="--ninja-executable=' + out_dir + '/third_party/ninja/ninja"'])
      api.run(api.step, 'gn clean',
              cmd=[gn, 'clean', out_dir])
      with api.context(cwd=out_dir, env={'CC': '/usr/local/bin/clang', 'CXX': '/usr/local/bin/clang++'}):
        api.run(api.step, 'cmake generate', cmd=['cmake', '-G"Unix Makefiles"', '.'])
        api.run(api.step, 'cmake build', cmd=['cmake', '--build', '.', '--parallel', '8'])

      if ccache:
        api.run(api.step, 'ccache stats-end', cmd=[ccache, '-s'])

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
