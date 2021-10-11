release:
	- rm -rf build/
	mkdir build
	bazel build //src:hello-world-wasm --compilation_mode opt
	cp bazel-bin/src/hello-world-wasm/hello-world.js build/hello-world.js
	cp bazel-bin/src/hello-world-wasm/hello-world.wasm build/hello-world.wasm

serve:
	python3 serve.py