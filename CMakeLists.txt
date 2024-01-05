cmake_minimum_required (VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project ("lycore" VERSION 0.1 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message("Build ${PROJECT_NAME} with ${CMAKE_BUILD_TYPE}")

set(CMAKE_EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")

######################################### Requirments Begin ########################################
# add_subdirectory("3rdparty")
add_subdirectory("3rdparty/yaml-cpp")
include_directories("3rdparty/yaml-cpp/include")
add_subdirectory("3rdparty/googletest")
include_directories("3rdparty/googletest/googletest/include")
include_directories("3rdparty/googletest/googlemock/include")
add_subdirectory("3rdparty/fmt")
include_directories("3rdparty/fmt/include")
add_subdirectory("3rdparty/expected")
include_directories("3rdparty/expected/include")
add_subdirectory("3rdparty/range-v3")
include_directories("3rdparty/range-v3/include")
######################################### Requirments End   ########################################

######################################### Library Begin ############################################
set(LIB_DIR "lib")
set(FFMPEG_DIR "${LIB_DIR}/ffmpeg")

######################################### Library End   ############################################
find_package(OpenSSL COMPONENTS SSL Crypto REQUIRED)
if (OPENSSL_FOUND)
    add_definitions(-DENABLE_OPENSSL)
else()
    message(FATAL_ERROR "OpenSSL not found")
endif()
set(FFMPEG_LIBRARIES_DIR "${PROJECT_SOURCE_DIR}/lib/ffmpeg/lib")
set(FFMPEG_INCLUDE "${PROJECT_SOURCE_DIR}/lib/ffmpeg/include")
set(FFMPEG_LIBRARIES "avcodec" "avformat" "avutil" "avdevice" "avfilter" "postproc" "swresample" "swscale")

#find_package(Qt5 REQUIRED COMPONENTS Widgets Gui Core Sql REQUIRED)

file(GLOB_RECURSE
    CoreSrcs
        CONFIGURE_DEPENDS
        "src/core/net/*.cpp"
        "src/core/util/*.cpp"
        "src/core/av/*.cpp"
        "src/core/config/*.cpp"
)
message(STATUS "SRC FILES: ${CoreSrcs}")

add_subdirectory(test)

#add_library(${PROJECT_NAME} STATIC ${CoreSrcs})
add_executable (${PROJECT_NAME} "src/main.cpp" ${CoreSrcs})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${FFMPEG_INCLUDE}"
)
target_link_directories(${PROJECT_NAME}
    PUBLIC
        ${FFMPEG_LIBRARIES_DIR}
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        fmt::fmt
        yaml-cpp::yaml-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        ${FFMPEG_LIBRARIES}
        range-v3
        # tl::expected
)

target_compile_features(${PROJECT_NAME}
    PUBLIC
        cxx_std_17
)
