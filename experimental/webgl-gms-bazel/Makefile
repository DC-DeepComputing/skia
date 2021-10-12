release:
	bazel build src:gm-bindings-wasm --compilation_mode opt --jobs 10
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/experimental/webgl-gms-bazel/src/gm-bindings-wasm/gm-bindings.js build/gm-bindings.js
	cp ../../bazel-bin/experimental/webgl-gms-bazel/src/gm-bindings-wasm/gm-bindings.wasm build/gm-bindings.wasm

debug:
	bazel build src:gm-bindings-wasm --compilation_mode dbg --jobs 10
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/experimental/webgl-gms-bazel/src/gm-bindings-wasm/gm-bindings.js build/gm-bindings.js
	cp ../../bazel-bin/experimental/webgl-gms-bazel/src/gm-bindings-wasm/gm-bindings.wasm build/gm-bindings.wasm

serve:
	python3 ../../tools/serve_wasm.py