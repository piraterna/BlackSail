project(BlackSail)

add_executable(blacksail_exe
	main.c)

set_property(TARGET blacksail_exe PROPERTY OUTPUT_NAME blacksail)

target_include_directories(
	blacksail_exe
	PRIVATE
	"\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>")

target_compile_features(blacksail_exe PRIVATE c_std_11)

target_link_libraries(blacksail_exe PRIVATE blacksail_lib)
