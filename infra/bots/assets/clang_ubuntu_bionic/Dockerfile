# This is based off the official LLVM docker container
# https://github.com/llvm/llvm-project/blob/76fd4bf675b5ceeeca0e4e15cf15d89c7acf4947/llvm/utils/docker/debian10/Dockerfile
#
# This was ubuntu:bionic on Jan 10 2025.
# Found by running
# docker pull ubuntu:bionic && docker images --digests | grep ubuntu
FROM ubuntu@sha256:152dc042452c496007f07ca9127571cb9c29697f42acbfad72324b2bb2e43c98

# Install build dependencies of llvm.
# First, Update the apt's source list and include the sources of the packages.
RUN grep deb /etc/apt/sources.list | \
    sed 's/^deb/deb-src /g' >> /etc/apt/sources.list
# Install compiler, python, etc. We need clang and lld because otherwise we have issues compiling
# compiler-rt (it fails using the built-in ld).
#
# The versions were added after seeing what was available when this image was created on Jan 8 2025.
# Specifying the versions makes this Docker container comply with SLSA level 1.
RUN apt-get update && \
    apt-get install -y --no-install-recommends  \
           ca-certificates=20230311ubuntu0.18.04.1 \
           gnupg=2.2.4-1ubuntu1.6 \
           build-essential=12.4ubuntu1 \
           make=4.1-9.1ubuntu1 \
           python3=3.6.7-1~18.04 \
           zlib1g=1:1.2.11.dfsg-0ubuntu2.2 \
           wget=1.19.4-1ubuntu2.2 \
           unzip=6.0-21ubuntu1.2 \
           git=1:2.17.1-1ubuntu0.18 \
           clang=1:6.0-41~exp5~ubuntu1 \
           lld=1:6.0-41~exp5~ubuntu1 \
           ninja-build=1.8.2-1 && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir /tmp/cmake && cd /tmp/cmake && \
    wget https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4-linux-x86_64.sh && \
    sh ./cmake-3.13.4-linux-x86_64.sh --skip-license && \
    cp /tmp/cmake/bin/* /usr/bin && \
    cp -r /tmp/cmake/share/* /usr/share

ENV TARGET_DIR=/tmp/clang_output
ENV CLANG_RELEASE=llvmorg-15.0.1

RUN mkdir -p /tmp/clang && cd /tmp/clang && \
    git clone --depth 1 -b ${CLANG_RELEASE} https://llvm.googlesource.com/llvm-project

# Check out the IWYU branch corresponding to our CLANG_RELEASE.
RUN git clone https://github.com/include-what-you-use/include-what-you-use.git /tmp/iwyu && \
    cd /tmp/iwyu && \
    git checkout clang_15

WORKDIR /tmp/clang/llvm-project

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

# https://libcxx.llvm.org/BuildingLibcxx.html#bootstrapping-build
# https://github.com/include-what-you-use/include-what-you-use#how-to-build-as-part-of-llvm
# This will build clang first and then use that new clang to build the runtimes and the
# iwyu probject.
RUN mkdir ${TARGET_DIR} out && \
    cmake -G Ninja -S llvm -B out \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${TARGET_DIR} \
    -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
    -DLLVM_USE_LINKER=lld \
    -DLLVM_ENABLE_UNWIND_TABLES=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLLVM_EXTERNAL_PROJECTS=iwyu \
    -DLLVM_EXTERNAL_IWYU_SOURCE_DIR=/tmp/iwyu

RUN ninja -C out install
RUN cp out/bin/llvm-symbolizer out/bin/llvm-profdata out/bin/llvm-cov ${TARGET_DIR}/bin
RUN cp `c++ -print-file-name=libstdc++.so.6` ${TARGET_DIR}/lib

# Use the newly compiled clang to build TSAN and MSAN libraries.
ENV CC=${TARGET_DIR}/bin/clang
ENV CXX=${TARGET_DIR}/bin/clang++

# It is very important to start the build from the runtimes subfolder and not the llvm subfolder
# like we did above when following the bootstrapping-build instructions.
# https://stackoverflow.com/a/73827100/1447621
RUN mkdir tsan_out && \
    cmake -G Ninja -S runtimes -B tsan_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_USE_SANITIZER=Thread

RUN ninja -C tsan_out cxx cxxabi
RUN cp -r tsan_out/lib ${TARGET_DIR}/tsan

# We would be following the instructions from
# https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
# but those are currently out of date (https://github.com/google/sanitizers/issues/1574)
RUN mkdir msan_out && \
    cmake -GNinja -S runtimes -B msan_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins

RUN ninja -C msan_out cxx cxxabi

RUN cp -r msan_out/lib ${TARGET_DIR}/msan