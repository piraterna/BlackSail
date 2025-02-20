project(liblacksail)

find_package(OpenSSL)

add_library(
    blacksail_lib SHARED
    bencode.c
    sha1.c
)

set_target_properties(blacksail_lib PROPERTIES OUTPUT_NAME "blacksail")

target_link_libraries(blacksail_lib ${OPENSSL_CRYPTO_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})

target_include_directories(
    blacksail_lib
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

target_include_directories(
	blacksail_lib
	PRIVATE
	"\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
    ${OPENSSL_INCLUDE_DIR})

target_compile_features(blacksail_lib PUBLIC c_std_11)