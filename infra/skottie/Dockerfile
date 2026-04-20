FROM gcr.io/skia-public/skottie-base@sha256:a835580afc8eb70ddb4dcdaf7b4dad5abc2e8d3a182b2126ea4098aca88d9bc6

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]