debug:
	bazelisk build //experimental/wasm-hello-world:hello_world_wasm --sandbox_base=/dev/shm --spawn_strategy=local
	- rm -rf build/
	cp -r ../../bazel-bin/experimental/wasm-hello-world/hello_world_wasm/ build/

serve:
	echo "Go check out http://localhost:8000/hello_world.html"
	python3 ../../tools/serve_wasm.py

