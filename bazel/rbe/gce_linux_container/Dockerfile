#FROM debian:bullseye-slim
FROM debian@sha256:40f90ead34f61f38c69fd604878c73c75ec944055328b9883f3b643c1b8886f0
RUN apt-get update && \
    apt-get install -y clang openjdk-11-jdk-headless
ENV JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"