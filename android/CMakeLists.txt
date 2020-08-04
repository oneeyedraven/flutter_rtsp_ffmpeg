cmake_minimum_required(VERSION 3.4.1)

set(lib_src_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
include_directories(
     ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)

add_library(avcodec-lib SHARED IMPORTED)
set_target_properties(avcodec-lib PROPERTIES IMPORTED_LOCATION
                      ${lib_src_DIR}/libavcodec.so)

add_library(avformat-lib SHARED IMPORTED)
set_target_properties(avformat-lib PROPERTIES IMPORTED_LOCATION
                     ${lib_src_DIR}/libavformat.so)

add_library(avutil-lib SHARED IMPORTED)
set_target_properties(avutil-lib PROPERTIES IMPORTED_LOCATION
                     ${lib_src_DIR}/libavutil.so)

add_library(swresample-lib SHARED IMPORTED)
set_target_properties(swresample-lib PROPERTIES IMPORTED_LOCATION
                     ${lib_src_DIR}/libswresample.so)

add_library(swscale-lib SHARED IMPORTED)
set_target_properties(swscale-lib PROPERTIES IMPORTED_LOCATION
                     ${lib_src_DIR}/libswscale.so)

#add_library(avdevice-lib SHARED IMPORTED)
#set_target_properties(swscale-lib PROPERTIES IMPORTED_LOCATION
#        ${lib_src_DIR}/libavdevice.so)

add_library(rtsp
            SHARED
            src/main/cpp/native-lib.cpp)

find_library(log-lib
                log)

target_link_libraries(rtsp
                        ${log-lib}
                        android # Add this.
                        avcodec-lib
                        avformat-lib
                        avutil-lib
                        swresample-lib
                        swscale-lib)
