FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

# deb-src is not included by default in the docker image. We support both the
# legacy /etc/apt/sources.list and the new deb822 /etc/apt/sources.list.d/debian.sources format.
RUN if [ -f /etc/apt/sources.list ]; then \
        cp /etc/apt/sources.list /etc/apt/sources.list~ && \
        sed -i 's/\(^deb\)\(.*\)$/\1\2\n\1-src \2/' /etc/apt/sources.list; \
    elif [ -f /etc/apt/sources.list.d/debian.sources ]; then \
        cp /etc/apt/sources.list.d/debian.sources /etc/apt/sources.list.d/debian.sources~ && \
        sed -i 's/Types: deb/Types: deb deb-src/' /etc/apt/sources.list.d/debian.sources; \
    fi

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y wget curl clang rsync jq python3-pip python3-setuptools python3-wheel && \
    apt-get build-dep -y mesa && \
    apt-get remove -y meson ninja-build && \
    pip3 install --break-system-packages meson ninja && \
    rm -rf /var/lib/apt/lists/*

COPY ./build_mesa.sh /opt/build_mesa.sh

# Add entry point so we can run the container w/o having to specifying the
# script name.
ENTRYPOINT ["/opt/build_mesa.sh"]
