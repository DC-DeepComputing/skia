# This image was created with the following 2 docker commands:
# FROM debian:bookworm-slim
#RUN apt-get update && \
#    apt-get install -y clang openjdk-11-jdk-headless \
#        fonts-dejavu-extra fonts-noto-core fonts-noto-cjk
#        fonts-noto-color-emoji
#
# Then, after seeing what the hash was of the base image and what versions of clang and the JDK
# were installed, those versions were pinned and the image was rebuilt and pushed to make sure
# those precise versions were used. This is to reach at least SLSA level 1 in that we know exactly
# what versions of the binaries are installed on the images we used to build things.
#
# Package Notes:
#   clang, openjdk: Bazel needs a default C++ compiler and Java runtime, but this will not be used
#                   to build Skia code.
#   fonts-dejavu-extra: The default fonts-dejavu-core do not have an italic font
#   fonts-noto-*: Coverage for many gms that test fonts
FROM debian@sha256:06ff0f4701c89b7ee0eab243f0c0791f28a2402e18e716d3d587b214b52e2857
RUN apt-get update && \
    apt-get install -y clang=1:14.0-55.2+b1 openjdk-11-jdk-headless=11.0.16+8-1 \
        fonts-dejavu-extra=2.37-2 fonts-noto-core=20201225-1 fonts-noto-cjk=1:20220127+repack1-1 \
        fonts-noto-color-emoji=2.038-1
ENV JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"