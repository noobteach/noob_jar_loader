cmake_minimum_required(VERSION 4.2)
project(any_jar_loader)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Release)

if (MSVC)
    add_compile_options(/utf-8)
    if (WIN32)
        add_link_options(/STACK:0x8000000)
        add_link_options(/ENTRY:special_entry_point)
    endif ()
endif ()

add_library(any_jar_loader SHARED
        get_jar_all_class_on_url.cpp
        get_thread_ctx_cl.cpp
        jni_show_io_gui.cpp
        resolve_class_depends.cpp
        special_entry_point.cpp
        thread_chooser_gui.cpp
)

set(JDK_HOME "C:/Program Files/Microsoft/jdk-25.0.3.9-hotspot")

target_include_directories(any_jar_loader PUBLIC
        ${JDK_HOME}/include
        ${JDK_HOME}/include/win32
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(any_jar_loader PUBLIC
        ${JDK_HOME}/lib
)

target_link_libraries(any_jar_loader PUBLIC
        jvm
        vcruntime
)