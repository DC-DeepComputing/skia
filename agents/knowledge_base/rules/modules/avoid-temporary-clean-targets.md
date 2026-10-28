---
name: skia/modules/avoid-temporary-clean-targets
description: When modifying Makefiles or build scripts, do not commit temporary clean or debug targets introduced during local troubleshooting once the underlying build issue is resolved
rubric: readability
severity: should_fix
---

**Rule.** Do not commit ad-hoc clean or debug targets (such as `clean_<target>`)
in Makefiles or build scripts that were introduced to work around stale
artifacts during troubleshooting. Rely on standard clean and build targets, and
remove temporary troubleshooting recipes before landing.

**Why.** One-off clean targets duplicate existing clean recipes, clutter build
scripts, and risk bitrotting once the specific build issue is resolved.

## Examples

Bad:

```cpp
clean_bidi:
	- rm -rf ../../out/canvaskit_wasm
	- rm -rf ./npm_build/bin
	- rm -rf ./build/
	$(MAKE) release_bidi
```

Good:

```cpp
release_bidi:
	# Does an incremental build where possible.
	./compile.sh client_bidi no_paragraph no_skottie
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/
```
