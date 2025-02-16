cmake_minimum_required(VERSION 3.14)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
  )
endif()

project(
    BlackSail
    VERSION 0.1.0
    DESCRIPTION "A fast, lightweight, and open-source BitTorrent client built for speed, privacy and control."
    HOMEPAGE_URL "https://github.com/piraterna/BlackSail"
    LANGUAGES C
)

add_subdirectory(lib)
add_subdirectory(src)