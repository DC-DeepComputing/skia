.PHONY: all

SOURCES = ../../../src/base/SkSemaphore.cpp \
          ../../../src/base/SkContainers.cpp \
		  ../../../src/base/SkThreadID.cpp \
          ../../../src/base/SkUTF.cpp \
		  ../../../src/base/SkUtils.cpp \
		  ../../../src/base/SkMalloc.cpp \
		  ../../../src/base/SkSafeMath.cpp \
          ../../../src/core/SkString.cpp \
		  ../../../src/base/SkTime.cpp \
          ../../../src/ports/SkDebug_stdio.cpp \
          ../../../src/ports/SkMemory_malloc.cpp

.FORCE:

bridge: .FORCE
	/usr/bin/clang++ -o ../../../out/Release/libbridge.a ../cpp/*.cpp $(SOURCES) -B -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L../../../out/Release

extract_info: bridge
	go build -a -o ../../../out/Release/extract_info extract_info/main.go

download_wiki: bridge
	go build -a -o ../../../out/Release/download_wiki download_wiki/main.go

.PHONY: generate_table
generate_table: bridge
	go build -a -o ../../../out/Release/generate_table generate_table/main.go
