.PHONY: all

ALL_HEADERS = ../../../include/core/SkSpan.h \
		  ../../../include/private/base/SkSemaphore.h \
	  	  ../../../include/private/base/SkContainers.h \
   		  ../../../include/private/base/SkThreadID.h \
		  ../../../src/base/SkUTF.h \
		  ../../../src/base/SkUtils.h \
		  ../../../include/core/SkString.h \
		  ../../../src/core/SkTime.h \
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
		  ../../../src/core/SkTime.cpp \
          ../../../src/ports/SkDebug_stdio.cpp \
          ../../../src/ports/SkMemory_malloc.cpp

bridge.a:
	/usr/bin/clang++ -o libbridge.a ../cpp/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L/usr/local/google/home/jlavrova/Sources/skia/out/Release

extract: bridge.a
	go build -o extract extract_info.go bridge.go helpers.go

download_wiki: bridge.a
	go build -o download_wiki download_wiki.go bridge.go helpers.go

generate_table:
	go build -o generate_table generate_table.go helpers.go