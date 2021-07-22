# Dockerfile for building Skia in release mode, using CMake.
FROM launcher.gcr.io/google/debian10
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python2 \
  python3 \
  curl \
  build-essential \
  cmake \
  libfreetype6-dev \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libxi-dev \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 --home /workspace/__cache skia

RUN cd /opt \
 && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

ENV PATH="/opt/depot_tools:${PATH}"
