cmake_minimum_required(VERSION 3.13)

project(zupedils VERSION 1.0)

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})
set(LIBRARIES ${LIBRARIES} ${GTK3_LIBRARIES})

add_executable(zupedils ${PROJECT_SOURCE_DIR}/zupedils.c
  ${PROJECT_SOURCE_DIR}/resources.c)
target_link_libraries(zupedils ${LIBRARIES})

install(TARGETS zupedils DESTINATION bin)

