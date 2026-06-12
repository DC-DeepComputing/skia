.PHONY: all gn-pa gn-no-pa cmake-pa cmake-no-pa bazel-pa bazel-no-pa

all: gn-pa gn-no-pa cmake-pa cmake-no-pa bazel-pa bazel-no-pa

gn-pa:
	gn gen out/pa_on --args='skia_use_partition_alloc=true'
	ninja --quiet -C out/pa_on

gn-no-pa:
	gn gen out/pa_off --args='skia_use_partition_alloc=false'
	ninja --quiet -C out/pa_off

cmake-pa:
	gn gen out/cmake_pa_on --args='skia_use_partition_alloc=true' --ide=json --json-ide-script=../../gn/gn_to_cmake.py
	ninja --quiet -C out/cmake_pa_on third_party/externals/partition_alloc/src/partition_alloc:buildflags
	cmake -B out/cmake_pa_on -S out/cmake_pa_on
	cmake --build out/cmake_pa_on --parallel

cmake-no-pa:
	gn gen out/cmake_pa_off --args='skia_use_partition_alloc=false' --ide=json --json-ide-script=../../gn/gn_to_cmake.py
	cmake -B out/cmake_pa_off -S out/cmake_pa_off
	cmake --build out/cmake_pa_off --parallel

bazel-pa:
	@echo "PartitionAlloc is not supported in Bazel yet."
	@exit 1

bazel-no-pa:
	bazel build //:core
