FROM gcr.io/skia-public/shaders-base@sha256:66ef19d82393a1039ed1eb72b9e68517ba86c2fa32d9922d8e4faa8804cdacfe

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]