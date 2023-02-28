.DEFAULT_GOAL := test
.PHONY: build test
FORCE:

ALL_HEADERS := $(wildcard ../../include/*.hpp) $(wildcard ../../../c/include/*.h)

CXX?=g++

$(ALL_HEADERS):

../../../../../target/debug/libicu_capi_staticlib.a: FORCE
	cargo build -p icu_capi_staticlib --features cpp_default

debug: FORCE
	cd ../externals/icu4x && \
	cargo build -p icu_capi_staticlib --features cpp_default --target-dir ../../icu4x/out && \
	cd ../../icu4x


