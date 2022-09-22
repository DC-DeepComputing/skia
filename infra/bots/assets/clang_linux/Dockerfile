# This is based off the official LLVM docker container
# https://github.com/llvm/llvm-project/blob/76fd4bf675b5ceeeca0e4e15cf15d89c7acf4947/llvm/utils/docker/debian10/Dockerfile
#
# This was launcher.gcr.io/google/debian10:latest on Sep 22 2022
# Found by running
# docker pull launcher.gcr.io/google/debian10:latest && docker images --digests | grep debian10
FROM launcher.gcr.io/google/debian10@sha256:3242ff21417c7722482c2085f86f28ed4f76cde00bf880f15fc1795975bc2a81

# Install build dependencies of llvm.
# First, Update the apt's source list and include the sources of the packages.
RUN grep deb /etc/apt/sources.list | \
    sed 's/^deb/deb-src /g' >> /etc/apt/sources.list
# Install compiler, python and subversion.
RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates gnupg \
           build-essential cmake make python3 zlib1g wget subversion unzip git && \
    rm -rf /var/lib/apt/lists/*
# Install a newer ninja release. It seems the older version in the debian repos
# randomly crashes when compiling llvm.
RUN wget "https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip" && \
    echo "d2fea9ff33b3ef353161ed906f260d565ca55b8ca0568fa07b1d2cab90a84a07 ninja-linux.zip" \
        | sha256sum -c  && \
    unzip ninja-linux.zip -d /usr/local/bin && \
    rm ninja-linux.zip

ENV TARGET_DIR=/tmp/clang_output
ENV CLANG_RELEASE=llvmorg-15.0.1

# Start Skia-specific needs, of compiling Clang from source
RUN mkdir -p /tmp/clang && cd /tmp/clang && \
    git clone --depth 1 -b ${CLANG_RELEASE} https://llvm.googlesource.com/llvm-project && \
    cd llvm-project && mkdir out && mkdir ${TARGET_DIR}

WORKDIR /tmp/clang/llvm-project/out

RUN cmake ../llvm -G Ninja \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_INSTALL_PREFIX=${TARGET_DIR} \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
    -DLLVM_ENABLE_TERMINFO=OFF

# TODO need clang-tools-extra;libcxx;libcxxabi;compiler-rt;lld

RUN ninja install

ENV TARGET_DIR2=/tmp/clang_output2
ENV CC=${TARGET_DIR}/bin/clang
ENV CXX=${TARGET_DIR}/bin/clang++

WORKDIR /tmp/clang/llvm-project

# https://libcxx.llvm.org/BuildingLibcxx.html
RUN mkdir ${TARGET_DIR2} && cmake -G Ninja -s llvm \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${TARGET_DIR2} \
    -DLLVM_ENABLE_PROJECTS="clang;lld" \
    -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi" \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
    -DLLVM_ENABLE_TERMINFO=OFF

RUN ninja install
#
#RUN cp bin/llvm-symbolizer bin/llvm-profdata bin/llvm-cov ${TARGET_DIR}/bin
#RUN cp `c++ -print-file-name=libstdc++.so.6` ${TARGET_DIR}/lib
