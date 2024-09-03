# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM gcr.io/skia-public/fiddler-base@sha256:19e37611d37a760f5b60a87bb26d467502d9bb05aa9d900c4f4823106640396c

# https://stackoverflow.com/a/44766666
COPY --chown=skia:skia . /tmp/skia

WORKDIR /tmp/skia/

ENV GIT_SYNC_DEPS_SKIP_EMSDK=1

# Fetch things we need but delete large third-party folders that we don't need for fiddler
RUN ./bin/fetch-gn \
  && ./bin/fetch-ninja \
  && ./tools/git-sync-deps \
  && rm -rf third_party/externals/dawn \
           third_party/externals/emsdk \
           third_party/externals/icu4x \
           third_party/externals/opengl-registry \
           third_party/externals/perfetto \
           third_party/externals/swiftshader \
           third_party/externals/unicodetools

RUN mkdir -p ./out/Static
RUN echo '  \n\
cc = "clang"  \n\
cxx = "clang++"  \n\
skia_use_egl = true  \n\
is_debug = false  \n\
skia_enable_fontmgr_fontconfig = true \n\
skia_use_perfetto = false \n\
skia_use_libgrapheme = false \n\
skia_use_icu4x = false \n\
skia_use_system_freetype2 = false  \n\
link_pool_depth=2  \n\
extra_cflags = [  \n\
  "-I/tmp/swiftshader/include",  \n\
  "-DGR_EGL_TRY_GLES3_THEN_GLES2",  \n\
  "-g0",  \n\
]  \n\
extra_ldflags = [  \n\
  "-L/usr/local/lib",  \n\
  "-Wl,-rpath",  \n\
  "-Wl,/usr/local/lib"  \n\
] ' > ./out/Static/args.gn

# Build Skia once so that incremental builds are much faster
RUN ./bin/gn gen out/Static
RUN git rev-parse HEAD > VERSION
RUN ./third_party/ninja/ninja -C out/Static fiddle

# Cleanup .git directories because they are not needed and take up space.
RUN find . -name .git -print0 | xargs -0 rm -rf
