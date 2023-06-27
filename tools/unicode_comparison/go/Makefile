.PHONY: all

ALL_HEADERS = ../../../include/core/SkSpan.h \
		  ../../../include/private/base/SkSemaphore.h \
	  	  ../../../include/private/base/SkContainers.h \
   		  ../../../include/private/base/SkThreadID.h \
		  ../../../src/base/SkUTF.h \
		  ../../../src/base/SkUtils.h \
		  ../../../include/core/SkString.h \
		  ../../../src/base/SkTime.h \
		  ../../../include/private/base/SkMalloc.h \
		  ../../../src/base/SkSafeMath.h \
		  ../../../include/private/base/SkFeatures.h

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

../../../out/Release/bridge.a:
	/usr/bin/clang++ -o ../../../out/Release/libbridge.a ../cpp/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L/usr/local/google/home/jlavrova/Sources/skia/out/Release

extract_info: ../../../out/Release/bridge.a
	go build -a -o ../../../out/Release/extract_info extract_info.go bridge.go helpers.go

download_wiki: ../../../out/Release/bridge.a
	go build -a -o ../../../out/Release/download_wiki download_wiki.go bridge.go helpers.go

.PHONY: generate_table
generate_table:
	go build -a -o ../../../out/Release/generate_table generate_table.go helpers.go