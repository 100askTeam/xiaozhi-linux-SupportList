file(GLOB_RECURSE SOURCES src/*.c)
add_library(lv_100ask_xz_ai STATIC ${SOURCES})

target_include_directories(lv_100ask_xz_ai SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
