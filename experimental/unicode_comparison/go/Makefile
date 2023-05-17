.PHONY: all

ALL_HEADERS := $(wildcard ../../../*.h)

$(ALL_HEADERS):

SOURCES = ../../../src/base/SkSemaphore.cpp \
          ../../../src/base/SkContainers.cpp \
		  ../../../src/base/SkThreadID.cpp \
          ../../../src/base/SkUTF.cpp \
		  ../../../src/base/SkUtils.cpp \
          ../../../src/core/SkString.cpp \
		  ../../../src/base/SkMalloc.cpp \
		  ../../../src/base/SkSafeMath.cpp \
          ../../../src/ports/SkDebug_stdio.cpp \
          ../../../src/ports/SkMemory_malloc.cpp

#all: extract_icu extract_icu4x extract_libgrapheme

bridge_icu.a:
	/usr/bin/clang++ -o libbridge.a ../bridge/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L../../../out/Debug

bridge_icu4x.a:
	/usr/bin/clang++ -o libbridge.a ../bridge/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L../../../out/Debug

bridge_libgrapheme.a:
	/usr/bin/clang++ -o libbridge.a ../bridge/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L../../../out/Debug

extract_icu: bridge_icu.a
	go build -o extract_icu extract_info.go bridge.go

extract_icu4x: bridge_icu4x.a
	go build -o extract_icu4x extract_info.go bridge.go

extract_libgrapheme: bridge_libgrapheme.a
	go build -o extract_libgrapheme extract_info.go bridge.go