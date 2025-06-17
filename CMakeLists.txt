cmake_minimum_required(VERSION 3.16)

project(EECS482_P2)
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_library(libcpu STATIC IMPORTED)
if (APPLE)
    add_compile_options(-D_XOPEN_SOURCE -Wno-deprecated-declarations)
    set_target_properties(libcpu PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/libcpu_macos.o)
else ()
    set_target_properties(libcpu PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/libcpu.o)
endif ()

# Thread library build target (customize this with the source files for your thread library)
add_library(libthread file1.cpp file2.cpp)
add_custom_command(POST_BUILD TARGET libthread COMMAND ${CMAKE_CURRENT_LIST_DIR}/autotag.sh push)

# Add a build target for each test program
add_executable(app app.cpp)
target_link_libraries(app libcpu libthread ${CMAKE_DL_LIBS} Threads::Threads)
