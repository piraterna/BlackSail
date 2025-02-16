project(liblacksail)

add_library(
    blacksail_lib SHARED
    bencode.c
)

target_include_directories(
    blacksail_lib
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

target_include_directories(
	blacksail_lib
	PRIVATE
	"\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>")

target_compile_features(blacksail_lib PUBLIC c_std_11)