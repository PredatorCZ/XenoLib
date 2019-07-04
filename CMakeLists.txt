cmake_minimum_required(VERSION 3.3)

set (UNICODE TRUE)

include(build.cmake)

if (WIN32 AND UNICODE)
	message(STATUS "Compiling with wchar_t")
	add_definitions(-D_UNICODE -DUNICODE)
endif()