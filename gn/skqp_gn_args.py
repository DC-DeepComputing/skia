# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SkqpGnArgs = {
    'extra_cflags':                     '[ "-DSK_ENABLE_DUMP_GPU", "-DSK_BUILD_FOR_SKQP" ]',
    'skia_enable_fontmgr_android':      'false',
    'skia_enable_fontmgr_empty':        'true',
    'skia_enable_pdf':                  'false',
    'skia_enable_skshaper':             'false',
    'skia_enable_skottie':              'false',
    'skia_enable_skrive':               'false',
    'skia_enable_sktext':               'false',
    'skia_enable_tools':                'true',
    'skia_enable_svg':                  'false',
    'skia_tools_require_resources':     'true',
    'skia_use_dng_sdk':                 'false',
    'skia_use_expat':                   'true',
    'skia_use_freetype':                'false',
    'skia_use_icu':                     'false',
    'skia_use_libgifcodec':             'false',
    'skia_use_libheif':                 'false',
    'skia_use_lua':                     'false',
    'skia_use_piex':                    'false',
    'skia_use_vulkan':                  'true',
    'skia_use_wuffs':                   'true',
}

SkqpBp_args = {
    'is_official_build':                'true',
    'target_cpu':                       '"none"',
    'target_os':                        '"android"',
    'is_debug':                         'false',
}

def GetGNArgs(arch, api_level, debug, ndk=None, is_android_bp=False):
    def gn_quote(s):
        return '"%s"' % s
    gn_args = SkqpGnArgs
    gn_args.update({
        'ndk_api':     api_level,
        'is_debug':    'true' if debug else 'false',
    })
    if is_android_bp is True:
        gn_args.update(SkqpBp_args)
    else:
        gn_args.update({
            'target_cpu':  gn_quote(arch),
            'ndk':         gn_quote(ndk),
            'ndk_api':     api_level,
        })
    return gn_args
