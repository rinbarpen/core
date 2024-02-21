message(STATUS "walk in examples")

file(GLOB_RECURSE UtilSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/util/*.cpp")
function(add_example_project ex_name src_path other_srcs includes libs)
    add_executable(${ex_name} ${src_path} ${UtilSrcs} ${other_srcs})
    target_include_directories(${ex_name}
        PRIVATE
            ${includes}
            "${PROJECT_SOURCE_DIR}/src/include"
            "3rdparty/yaml-cpp/include"
            "3rdparty/fmt/include"
    )
    target_link_libraries(${ex_name}
        PRIVATE
            ${libs}
            fmt::fmt
            yaml-cpp::yaml-cpp
            OpenSSL::SSL
            OpenSSL::Crypto
    )
    target_compile_features(${ex_name}
        PUBLIC
            cxx_std_17
    )

    message(STATUS "Add ${ex_name} to example")
endfunction()

file(GLOB_RECURSE NetSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/net/*.cpp")
file(GLOB_RECURSE MultiMediaSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/multimedia/*.cpp")
file(GLOB_RECURSE ArgsSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/args/*.cpp")

set(Srcs "${NetSrcs}" "${UtilSrcs}" "${MultiMediaSrcs} ${ArgsSrcs}")

add_example_project(args
    "${CMAKE_CURRENT_SOURCE_DIR}/args/arg.cc"
    "${ArgsSrcs}"
    ""
    "")
