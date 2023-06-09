WORKSPACE_DIR=../..
PATHKIT_BIN_DIR=${WORKSPACE_DIR}/out/pathkit

debug:
	./compile.sh debug
	- rm -rf build/
	mkdir  -p ./build/wasm ./build/asmjs
	cp ${PATHKIT_BIN_DIR}/pathkit.js     ./build/wasm/pathkit.js
	cp ${PATHKIT_BIN_DIR}/pathkit.wasm   ./build/wasm/pathkit.wasm

	./compile.sh asm.js debug
	cp ${PATHKIT_BIN_DIR}/pathkit.js     ./build/asmjs/pathkit.js

release:
	./compile.sh
	- rm -rf build/
	mkdir  -p ./build/wasm ./build/asmjs
	cp ${PATHKIT_BIN_DIR}/pathkit.js     ./build/wasm/pathkit.js
	cp ${PATHKIT_BIN_DIR}/pathkit.wasm   ./build/wasm/pathkit.wasm

	./compile.sh asm.js
	cp ${PATHKIT_BIN_DIR}/pathkit.js     ./build/asmjs/pathkit.js
	cp ${PATHKIT_BIN_DIR}/pathkit.js.mem ./build/asmjs/pathkit.js.mem

npm:
	rm -rf ./npm-wasm/bin ./npm-asmjs/bin
	mkdir -p ./npm-wasm/bin ./npm-asmjs/bin
	./compile.sh
	cp ${PATHKIT_BIN_DIR}/pathkit.js   ./npm-wasm/bin
	cp ${PATHKIT_BIN_DIR}/pathkit.wasm ./npm-wasm/bin
	./compile.sh asm.js
	cp ${PATHKIT_BIN_DIR}/pathkit.js     ./npm-asmjs/bin/pathkit.js
	cp ${PATHKIT_BIN_DIR}/pathkit.js.mem ./npm-asmjs/bin/pathkit.js.mem
	cp CHANGELOG.md ./npm-wasm
	cp CHANGELOG.md ./npm-asmjs

test-continuous:
	echo "Assuming npm ci has been run by user"
	echo "Also assuming make debug or make release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll

test-continuous-asmjs:
	echo "Assuming npm ci has been run by user"
	echo "Also assuming make debug or make release has also been run by a user (if needed)"
	ASM_JS=1 npx karma start ./karma.conf.js --no-single-run --watch-poll

local-example:
	echo "Go check out http://localhost:8000/npm-wasm/example.html"
	echo "or http://localhost:8000/npm-asmjs/example.html"
	python3 ../../tools/serve_wasm.py
