FROM gcr.io/skia-public/debugger-app-base@sha256:64a03674b36e4753ed6de899a3f8e105a9344d2335018f64d3af8b3778456aa8

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]