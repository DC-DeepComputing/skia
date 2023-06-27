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

bridge_icu.a:
	/usr/bin/clang++ -o libbridge_icu.a ../cpp/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L/usr/local/google/home/jlavrova/Sources/skia/out/Release

bridge_icu4x.a:
	/usr/bin/clang++ -o libbridge_icu4x.a ../cpp/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode_icu4x.a -l:libicu_capi_staticlib.a -I../../.. -L/usr/local/google/home/jlavrova/Sources/skia/out/Release

bridge_libgrapheme.a:
	/usr/bin/clang++ -o libbridge_libgrapheme.a ../cpp/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L/usr/local/google/home/jlavrova/Sources/skia/out/Release

extract_icu: bridge_icu.a
	go build -o extract_icu extract_info.go bridge_icu.go bridge.go helpers.go

extract_icu4x: bridge_icu4x.a
	go build -o extract_icu4x extract_info.go bridge_icu4x.go bridge.go helpers.go

extract_libgrapheme: bridge_libgrapheme.a
	go build -o extract_libgrapheme extract_info.go bridge_libgrapheme.go bridge.go helpers.go

download_wiki: bridge_icu.a
	go build -o download_wiki download_wiki.go bridge_icu.go bridge.go helpers.go

generate_table:
	go build -o generate_table generate_table.go helpers.go