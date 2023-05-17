.PHONY: all

ALL_HEADERS = ../../../include/core/SkSpan.h \
		  ../../../include/private/base/SkSemaphore.h \
	  	  ../../../include/private/base/SkContainers.h \
   		  ../../../include/private/base/SkThreadID.h \
		  ../../../src/base/SkUTF.h \
		  ../../../src/base/SkUtils.h \
		  ../../../include/core/SkString.h \
		  ../../../include/private/base/SkMalloc.h \
		  ../../../src/base/SkSafeMath.h \
		  ../../../include/private/base/SkFeatures.h

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

bridge_icu.a:
	/usr/bin/clang++ -o libbridge.a ../bridge/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode_icu.a -I../../.. -L../libs

bridge_icu4x.a:
	/usr/bin/clang++ -o libbridge.a ../bridge/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode_icu4x.a -I../../.. -L../libs

bridge_libgrapheme.a:
	/usr/bin/clang++ -o libbridge.a ../bridge/*.cpp $(SOURCES) -std=c++20 -O3 -Wall -Wextra -fPIC -shared -l:libskunicode_libgrapheme.a -I../../.. -L../libs

extract_icu: bridge_icu.a
	go build -o extract_icu extract_info.go bridge.go

extract_icu4x: bridge_icu4x.a
	go build -o extract_icu4x extract_info.go bridge.go

extract_libgrapheme: bridge_libgrapheme.a
	go build -o extract_libgrapheme extract_info.go bridge.go