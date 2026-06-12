.PHONY: all gn-pa gn-no-pa cmake-pa cmake-no-pa bazel-pa bazel-no-pa

all: gn-pa gn-no-pa cmake-pa cmake-no-pa bazel-pa bazel-no-pa

gn-pa:
	gn gen out/pa_on --args='skia_use_partition_alloc=true'
	ninja --quiet -C out/pa_on

gn-no-pa:
	gn gen out/pa_off --args='skia_use_partition_alloc=false'
	ninja --quiet -C out/pa_off

cmake-pa cmake-no-pa bazel-pa bazel-no-pa:
	@echo "Per gerrit_rules.md, only GN/Ninja builds are supported; Bazel and CMake cannot be used."
	@exit 1
