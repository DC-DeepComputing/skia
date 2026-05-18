FROM gcr.io/skia-public/skottie-base@sha256:b6e0cb87c65e01f784e63fd9aa43742e4b4f1e397242ad82b40cee57d4c02474

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]