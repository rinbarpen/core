enable_testing()
message(STATUS "walk in testing")

set(GTEST_LIBS GTest::gtest_main GTest::gtest GTest::gmock_main GTest::gmock)

function(add_test_project test_name src_path other_srcs includes libs)
    add_executable(${test_name} ${src_path} ${other_srcs})
    target_include_directories(${test_name}
        PRIVATE
            "${includes}"
            "${GTEST_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}/src/include"
            "${PROJECT_SOURCE_DIR}/3rdparty/yaml-cpp/include"
            "${PROJECT_SOURCE_DIR}/3rdparty/fmt/include"
            "${PROJECT_SOURCE_DIR}/3rdparty/googletest/googletest/include"
            "${PROJECT_SOURCE_DIR}/3rdparty/googletest/googlemock/include"
    )
    target_link_libraries(${test_name}
        PRIVATE
            ${libs}
            ${GTEST_LIBS}
            fmt::fmt
            yaml-cpp::yaml-cpp
            OpenSSL::SSL
            OpenSSL::Crypto
    )
    target_compile_features(${test_name}
        PUBLIC
            cxx_std_17
            c_std_11
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
    message(STATUS "Add ${test_name} to test")
endfunction()

file(GLOB_RECURSE UtilSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/util/*.cpp")
file(GLOB_RECURSE NetSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/net/*.cpp" ${UtilSrcs})
file(GLOB_RECURSE MultiMediaSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/multimedia/*.cpp" ${UtilSrcs})
file(GLOB_RECURSE ArgsSrcs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/core/args/*.cpp" ${UtilSrcs})
set(Srcs "${NetSrcs} ${UtilSrcs} ${MultiMediaSrcs} ${ArgsSrcs}")

add_test_project(Test_Timer
    "${CMAKE_CURRENT_SOURCE_DIR}/core/util/timer/Timer.cxx"
    "${NetSrcs}"
    ""
    "")
add_test_project(Test_Trans
    "${CMAKE_CURRENT_SOURCE_DIR}/core/util/Trans.cxx"
    "${UtilSrcs}"
    ""
    "")
add_test_project(Test_OSUtil
    "${CMAKE_CURRENT_SOURCE_DIR}/core/util/OSUtil.cxx"
    "${UtilSrcs}"
    ""
    "")
