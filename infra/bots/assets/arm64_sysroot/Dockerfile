# This was ubuntu:noble on Jan 8 2025.
# Found by running
# docker pull ubuntu:noble && docker images --digests | grep ubuntu
FROM ubuntu@sha256:80dd3c3b9c6cecb9f1667e9290b3bc61b78c2678c02cbdae5f0fea92cc6734ab

# The versions were added after seeing what was available when this image was created on Jan 8 2025.
# Specifying the versions makes this Docker container comply with SLSA level 1.
RUN apt-get update && \
    apt-get install -y --no-install-recommends  \
    libstdc++-12-dev-arm64-cross \
    libgcc-12-dev-arm64-cross \
    binutils-arm-linux-gnueabihf \
    && rm -rf /var/lib/apt/lists/*

ENV TARGET_DIR=/tmp/arm64_sysroot_output
RUN cp -RL /usr/aarch64-linux-gnu ${TARGET_DIR} && \
    cp -RL /usr/lib/gcc-cross/aarch64-linux-gnu/12 ${TARGET_DIR}/gcc-cross && \
    cp /usr/lib/x86_64-linux-gnu/libbfd-2.42-armhf.so ${TARGET_DIR}/lib && \
    cp /usr/lib/x86_64-linux-gnu/libopcodes-2.42-armhf.so ${TARGET_DIR}/lib && \
    cp /usr/lib/x86_64-linux-gnu/libctf-armhf.so.0.0.0 ${TARGET_DIR}/lib
RUN sed -i 's/\/usr\/aarch64-linux-gnu\/lib\///g' ${TARGET_DIR}/lib/libc.so && \
    sed -i 's/\/usr\/aarch64-linux-gnu\/lib\///g' ${TARGET_DIR}/lib/libm.so
