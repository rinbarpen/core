cmake_minimum_required (VERSION 3.4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CoreProjectName "lycore")

project (${CoreProjectName} VERSION 0.1 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message("Build ${CoreProjectName} with ${CMAKE_BUILD_TYPE}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Gcc specific flags
#    list(APPEND CMAKE_CXX_FLAGS "-fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang specific flags
#    list(APPEND CMAKE_CXX_FLAGS "-fPIC")
#    list(APPEND CMAKE_CXX_FLAGS "-stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -stdlib=libc++")
elseif (MSVC)
    # MSVC specific flags
#    list(APPEND CMAKE_CXX_FLAGS "/MP")
#    list(APPEND CMAKE_CXX_FLAGS "/permissive-")
#    list(APPEND CMAKE_CXX_FLAGS "/sdl-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /permissive- /sdl-")
endif()

######################################### Requirments Begin ########################################
add_subdirectory("3rdparty/yaml-cpp")
include_directories("3rdparty/yaml-cpp/include")
add_subdirectory("3rdparty/googletest")
include_directories("3rdparty/googletest/googletest/include")
include_directories("3rdparty/googletest/googlemock/include")
add_subdirectory("3rdparty/fmt")
include_directories("3rdparty/fmt/include")
add_subdirectory("3rdparty/range-v3")
include_directories("3rdparty/range-v3/include")
add_subdirectory("3rdparty/glfw")
include_directories("3rdparty/glfw/include")
add_subdirectory("3rdparty/libyuv")
include_directories("3rdparty/libyuv/include")
######################################### Requirments End   ########################################

######################################### Library Begin ############################################
set(FFMPEG_DIR "${PROJECT_SOURCE_DIR}/3rdparty/ffmpeg")

find_package(OpenSSL COMPONENTS SSL Crypto REQUIRED)
if (OPENSSL_FOUND)
    add_definitions(-DENABLE_OPENSSL)
else()
    message(FATAL_ERROR "OpenSSL not found")
endif()

if (WIN32)
    set(SYSTEM_NAME "win")
    message(STATUS "This is Window platform")
elseif(UNIX)
    set(SYSTEM_NAME "linux")
    message(STATUS "This is Linux platform")
else()
    message(STATUS "Unknown platform")
endif()

if (WIN32)
    set(FFMPEG_LIBRARIES_DIR "${FFMPEG_DIR}/lib/${SYSTEM_NAME}")
else()
    set(FFMPEG_LIBRARIES_DIR "${FFMPEG_DIR}/lib")
endif()
set(FFMPEG_INCLUDE "${FFMPEG_DIR}/include")
set(FFMPEG_LIBRARIES
    "avcodec" "avformat" "avutil"
    "avdevice" "avfilter" "postproc"
    "swresample" "swscale")

#if (WIN32)
#    foreach(lib ${FFMPEG_LIBRARIES})
#        file(COPY "${FFMPEG_LIBRARIES_DIR}/${CMAKE_BUILD_TYPE}/${lib}.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${lib}.dll")
#        message(STATUS "Copy ${FFMPEG_LIBRARIES_DIR}/${CMAKE_BUILD_TYPE}/${lib}.dll to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${lib}.dll")
#    endforeach()
#elseif (UNIX)
#    foreach(lib ${FFMPEG_LIBRARIES})
#        file(COPY "${FFMPEG_LIBRARIES_DIR}/${lib}.so" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${lib}.so")
#        message(STATUS "Copy ${FFMPEG_LIBRARIES_DIR}/${lib}.so to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${lib}.so")
#    endforeach()
#endif()
######################################### Library End   ############################################

add_definitions(-DLOG_MORE_FUNCTION_INFO_ENABLED)
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#find_package(Qt5 REQUIRED COMPONENTS Widgets Gui Core Sql REQUIRED)
file(GLOB_RECURSE
    CoreSrcs
        CONFIGURE_DEPENDS
        "src/core/net/*.cpp"
        "src/core/util/*.cpp"
        "src/core/multimedia/*.cpp"
        "src/core/config/*.cpp"
)
message(STATUS "SOURCE FILES: ${CoreSrcs}")

#add_definitions(-DENABLE_TEST)
if (ENABLE_TEST)
    add_subdirectory(test)
endif()

# add_library(${PROJECT_NAME} STATIC ${CoreSrcs})
add_executable (${CoreProjectName} "src/main.cpp" ${CoreSrcs})

target_include_directories(${CoreProjectName}
    PUBLIC
        "${PROJECT_SOURCE_DIR}/src/include"
        "${FFMPEG_INCLUDE}"
        "3rdparty/yaml-cpp/include"
        "3rdparty/googletest/googletest/include"
        "3rdparty/googletest/googlemock/include"
        "3rdparty/fmt/include"
        "3rdparty/range-v3/include"
#        "3rdparty/sqlite3pp/include"
#        "3rdparty/glfw/include"
#        "3rdparty/libyuv/include"
)
target_link_directories(${CoreProjectName}
    PUBLIC
        ${FFMPEG_LIBRARIES_DIR}
)
target_link_libraries(${CoreProjectName}
    PUBLIC
        fmt::fmt
        yaml-cpp::yaml-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        ${FFMPEG_LIBRARIES}
        range-v3
        D3D9.lib
#        glfw
#        yuvconvert
)
target_compile_features(${CoreProjectName}
    PUBLIC
        cxx_std_17
)
