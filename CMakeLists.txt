cmake_minimum_required (VERSION 3.6)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CoreProjectName "lycore")

project (${CoreProjectName} VERSION 0.4 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message("Build ${CoreProjectName} with ${CMAKE_BUILD_TYPE}")

if (WIN32)
    set(SYSTEM_NAME "windows")
    message(STATUS "This is Window platform")
elseif(UNIX)
    set(SYSTEM_NAME "linux")
    message(STATUS "This is Linux platform")
else()
    message(STATUS "Unknown platform")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Gcc specific flags
#    list(APPEND CMAKE_CXX_FLAGS "-fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang specific flags
#    list(APPEND CMAKE_CXX_FLAGS "-fPIC")
#    list(APPEND CMAKE_CXX_FLAGS "-stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
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
add_subdirectory("3rdparty/fmt")
include_directories("3rdparty/fmt/include")
add_subdirectory("3rdparty/googletest")
include_directories("3rdparty/googletest/googletest/include")
include_directories("3rdparty/googletest/googlemock/include")
######################################### Requirments End   ########################################

######################################### Library Begin ############################################
set(FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ffmpeg")

find_package(OpenSSL COMPONENTS SSL Crypto REQUIRED)
if (OPENSSL_FOUND)
    add_definitions(-DENABLE_OPENSSL)
else()
    message(FATAL_ERROR "OpenSSL not found")
endif()

if (WIN32)
    set(FFMPEG_LIBRARIES_DIR "${FFMPEG_DIR}/lib/${SYSTEM_NAME}/${CMAKE_BUILD_TYPE}")
else()
    set(FFMPEG_LIBRARIES_DIR "${FFMPEG_DIR}/lib/${SYSTEM_NAME}")
endif()
set(FFMPEG_INCLUDE "${FFMPEG_DIR}/include")
set(FFMPEG_LIBRARIES
    "avcodec" "avformat" "avutil"
    "avdevice" "avfilter" "postproc"
    "swresample" "swscale")

######################################### Library End   ############################################

add_definitions(-DLOG_MORE_FUNCTION_INFO_ENABLED)

#find_package(Qt5 REQUIRED COMPONENTS Widgets Gui Core Sql REQUIRED)
file(GLOB_RECURSE
    CoreSrcs
        CONFIGURE_DEPENDS
        "src/core/net/*.cpp"
        "src/core/util/*.cpp"
        "src/core/multimedia/*.cpp"
        "src/core/config/*.cpp"
        "src/core/args/*.cpp"
)
# message(STATUS "SOURCE FILES: ${CoreSrcs}")

#add_definitions(-DENABLE_TEST)
#add_subdirectory(examples)
if (ENABLE_TEST)
    add_subdirectory(tests)
endif()

add_executable (${CoreProjectName} "src/main.cpp" ${CoreSrcs} "src/ScreenLive.cpp")
target_include_directories(${CoreProjectName}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
        "${FFMPEG_INCLUDE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/yaml-cpp/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fmt/include"
)
target_link_directories(${CoreProjectName}
    PUBLIC
        ${FFMPEG_LIBRARIES_DIR}
)
if(WIN32)
    target_link_libraries(${CoreProjectName}
        PUBLIC
            fmt::fmt
            yaml-cpp::yaml-cpp
            OpenSSL::SSL
            OpenSSL::Crypto
            ${FFMPEG_LIBRARIES}
            D3D9.lib
    )
elseif(UNIX)
    target_link_libraries(${CoreProjectName}
        PUBLIC
            fmt::fmt
            yaml-cpp::yaml-cpp
            OpenSSL::SSL
            OpenSSL::Crypto
            ${FFMPEG_LIBRARIES}
    )
endif()
target_compile_features(${CoreProjectName}
    PUBLIC
        cxx_std_17
        c_std_11
)
