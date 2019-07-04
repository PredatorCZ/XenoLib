
if (NOT DEFINED XenoLib_MASTER_DIR)
    set(XenoLib_MASTER_DIR .)
endif()

add_library(XenoLib STATIC 
"${XenoLib_MASTER_DIR}/3rd_party/libpng/png.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngerror.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngget.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngmem.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngpread.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngread.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngrio.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngrtran.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngrutil.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngset.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngtest.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngtrans.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngwio.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngwrite.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngwtran.c"
"${XenoLib_MASTER_DIR}/3rd_party/libpng/pngwutil.c"
"${XenoLib_MASTER_DIR}/3rd_party/precore/datas/MasterPrinter.cpp" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/adler32.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/crc32.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/deflate.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/inffast.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/inflate.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/inftrees.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/trees.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/uncompr.c" 
"${XenoLib_MASTER_DIR}/3rd_party/zlib/zutil.c" 
"${XenoLib_MASTER_DIR}/source/BC.cpp" 
"${XenoLib_MASTER_DIR}/source/DRSM.cpp" 
"${XenoLib_MASTER_DIR}/source/LBIM.cpp" 
"${XenoLib_MASTER_DIR}/source/MTHS.cpp" 
"${XenoLib_MASTER_DIR}/source/MTXT.cpp" 
"${XenoLib_MASTER_DIR}/source/MXMD.cpp" 
"${XenoLib_MASTER_DIR}/source/PNGWrap.cpp" 
"${XenoLib_MASTER_DIR}/source/SAR.cpp" 
)

include_directories("${XenoLib_MASTER_DIR}/include/")
include_directories("${XenoLib_MASTER_DIR}/source/")
include_directories("${XenoLib_MASTER_DIR}/3rd_party/precore/")
include_directories("${XenoLib_MASTER_DIR}/3rd_party/libpng/")
include_directories("${XenoLib_MASTER_DIR}/3rd_party/zlib/")