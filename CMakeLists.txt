cmake_minimum_required(VERSION 3.15)

project(
  ReflectionPOC
  VERSION 0.0.1
  LANGUAGES C CXX
)

# --- Project and build options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Adjust paths 

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH} )
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH} )

# --- Install conan dependencies
if(CONAN_EXPORTED) # in conan local cache
  # standard conan installation, deps will be defined in conanfile.py
  # and not necessary to call conan again, conan is already running
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()

else ()
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
				  "${CMAKE_BINARY_DIR}/conan.cmake")
  endif()
  include(${CMAKE_BINARY_DIR}/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.txt
    BUILD missing
    BASIC_SETUP
    CMAKE_TARGETS
	  CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo;MinSizeRel"
    )
endif()


file(GLOB_RECURSE REFLECTION_SOURCE_FILES
    src/*.cpp
    src/*.c
)

file(GLOB_RECURSE REFLECTION_HEADER_FILES
    src/*.h
    src/*.hpp
    src/*.hxx
)

add_library(Reflection STATIC ${REFLECTION_SOURCE_FILES} ${REFLECTION_HEADER_FILES})

target_include_directories(Reflection INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(tests)