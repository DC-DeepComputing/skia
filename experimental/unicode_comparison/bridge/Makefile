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

all: main

bridge.a:
	/usr/bin/clang++ -o libbridge.a *.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L../../../out/Debug

main: bridge.a
	go build bridge.go

