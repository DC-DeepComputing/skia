release:
	- rm -rf build/
	mkdir build
	bazel build //src:hello-world.js --config=wasm --compilation_mode opt
	cp bazel-bin/src/hello-world.js build/hello-world.js
	cp bazel-bin/src/hello-world.wasm build/hello-world.wasm

serve:
	python3 serve.py