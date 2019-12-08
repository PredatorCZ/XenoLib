cmake_minimum_required(VERSION 3.3)

project(XenoLib)

if(NOT TARGETEX_LOCATION)
	set(TARGETEX_LOCATION 3rd_party/precore/cmake)
	include(${TARGETEX_LOCATION}/targetex.cmake)
	include(${TARGETEX_LOCATION}/chartype.cmake)
endif()

set(CMAKE_CXX_STANDARD 11)

build_target(
	TYPE STATIC
	SOURCES
		3rd_party/libpng/png.c
		3rd_party/libpng/pngerror.c
		3rd_party/libpng/pngget.c
		3rd_party/libpng/pngmem.c
		3rd_party/libpng/pngpread.c
		3rd_party/libpng/pngread.c
		3rd_party/libpng/pngrio.c
		3rd_party/libpng/pngrtran.c
		3rd_party/libpng/pngrutil.c
		3rd_party/libpng/pngset.c
		3rd_party/libpng/pngtest.c
		3rd_party/libpng/pngtrans.c
		3rd_party/libpng/pngwio.c
		3rd_party/libpng/pngwrite.c
		3rd_party/libpng/pngwtran.c
		3rd_party/libpng/pngwutil.c
		3rd_party/precore/datas/MasterPrinter.cpp 
		3rd_party/zlib/adler32.c 
		3rd_party/zlib/crc32.c 
		3rd_party/zlib/deflate.c 
		3rd_party/zlib/inffast.c 
		3rd_party/zlib/inflate.c 
		3rd_party/zlib/inftrees.c 
		3rd_party/zlib/trees.c 
		3rd_party/zlib/uncompr.c 
		3rd_party/zlib/zutil.c 
		source/BC.cpp 
		source/DRSM.cpp 
		source/LBIM.cpp 
		source/MTHS.cpp 
		source/MTXT.cpp 
		source/MXMD.cpp 
		source/PNGWrap.cpp 
		source/SAR.cpp 
	INCLUDES
		source
		include
		3rd_party/precore
		3rd_party/libpng
		3rd_party/zlib
		3rd_party/pugixml/src
		3rd_party/zstd/lib
		3rd_party/zstd/lib/common
	NO_PROJECT_H
	NO_VERINFO
)