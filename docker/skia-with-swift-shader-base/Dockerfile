# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM launcher.gcr.io/google/clang-debian9 AS build
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python \
  curl \
  build-essential \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev

ADD https://storage.googleapis.com/skia-swiftshader/libvk_swiftshader.so /usr/local/lib/libvk_swiftshader.so
RUN cd /tmp \
  && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git' \
  && git clone https://swiftshader.googlesource.com/SwiftShader swiftshader

RUN mkdir -m 0777 /skia
