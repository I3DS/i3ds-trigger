cmake_minimum_required(VERSION 2.8)

project (i3ds-trigger)

set (PROJECT_AUTHOR "SINTEF Digital")
set (PROJECT_URL "http://i3ds-h2020.eu/")
set (PROJECT_LICENSE "MPLv2")
set (PROJECT_VERSION "1.0")

option(BUILD_EMBEDDED "Build for embedded systems" OFF)

if (BUILD_EMBEDDED)
  if(NOT DEFINED CMAKE_FIND_ROOT_PATH)
    MESSAGE(FATAL_ERROR "CMAKE_FIND_ROOT_PATH path must be set when compiling for embedded")
  endif(NOT DEFINED CMAKE_FIND_ROOT_PATH)
  SET(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
  SET(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")
endif (BUILD_EMBEDDED)

find_package (Boost COMPONENTS program_options unit_test_framework log REQUIRED)

set (LIBS
  i3ds
  ${Boost_LIBRARIES}
  )

include_directories(BEFORE include)

set (SRCS
  src/trigger_driver.c
  )

add_executable (i3ds_trigger_demo src/trigger_demo.c ${SRCS})

add_executable (i3ds_trigger src/i3ds_trigger.c ${SRCS})
target_link_libraries(i3ds_trigger_demo ${LIBS})
