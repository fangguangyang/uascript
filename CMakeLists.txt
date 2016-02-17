cmake_minimum_required(VERSION 2.8.11)
project(uascript C)
#set(CMAKE_VERBOSE_MAKEFILE ON)

####################################
# Build Type and Compiler Settings #
####################################

# Set default build type.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE not given; setting to 'Debug'.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# compiler flags
if(CMAKE_COMPILER_IS_GNUCC OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang")
    add_definitions(-std=c99 -pipe -Wall -Wextra -Werror -Wformat -Wno-unused-parameter
                    -Wno-unused-function -Wno-unused-label -Wpointer-arith -Wreturn-type #-Wsign-compare
                    -Wmultichar -Wstrict-overflow -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes
                    -Winit-self -Wuninitialized -Wformat-security -Wformat-nonliteral
                    -Wno-unused-result -Wno-sign-compare)
  # library linking
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "") # cmake sets -rdynamic by default
  if(NOT WIN32 AND NOT CYGWIN)
	add_definitions(-Wshadow #-Wconversion
                    -fvisibility=hidden -fPIC)
    if(NOT APPLE)
      set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-z,norelro -Wl,--hash-style=gnu -Wl,--build-id=none")
    endif()
  endif()

  # Debug
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	#add_definitions(-fsanitize=address)
    #list(APPEND open62541_LIBRARIES asan)

	#add_definitions(-fsanitize=undefined)
    #list(APPEND open62541_LIBRARIES ubsan)

  elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR
         CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-ffunction-sections -fdata-sections -fno-stack-protector -fno-unwind-tables
                    -fno-asynchronous-unwind-tables -fno-math-errno -fmerge-all-constants -fno-ident)
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -s")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
    if(APPLE)
      set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-dead_strip")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-dead_strip")
    else()
      set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--gc-sections")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
      if(NOT WIN32 AND NOT CYGWIN)
        # these settings reduce the binary size by ~2kb
        set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-z,norelro -Wl,--hash-style=gnu -Wl,--build-id=none")
      endif()
    endif()
  endif()
  
  if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE=1")
  endif()
elseif(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

#################
# Build Targets #
#################

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/lua-5.2.4)

set(lib_sources ${PROJECT_SOURCE_DIR}/src/libua.c
                ${PROJECT_SOURCE_DIR}/src/libua_types.c
                ${PROJECT_SOURCE_DIR}/src/libua_server.c
                ${PROJECT_SOURCE_DIR}/src/libua_client.c
                ${PROJECT_SOURCE_DIR}/src/open62541.c)

file(GLOB executable_sources ${PROJECT_SOURCE_DIR}/src/lua-5.2.4/*.c)

add_library(ua SHARED ${lib_sources})

# remove compiler flags for lua
if(CMAKE_COMPILER_IS_GNUCC OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang")
  add_definitions(-Wno-cast-qual -Wno-format-nonliteral -Wno-strict-overflow)
endif()
add_executable(uascript ${lib_sources} ${executable_sources} ${PROJECT_SOURCE_DIR}/src/uascript.c)
target_link_libraries(uascript m)
