FROM gcr.io/skia-public/shaders-base@sha256:ff532331c6247a087d0143da2b5042be1bf843d3f28e6f494c7cdd5a25d29f53

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]