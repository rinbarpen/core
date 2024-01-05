enable_testing()
message(STATUS "walk in testing")

set(GTEST_LIBS GTest::gtest_main GTest::gtest GTest::gmock_main GTest::gmock)

function(add_test_project test_name src_path other_srcs includes libs)
    add_executable(${test_name} ${src_path} ${other_srcs})
    target_include_directories(${test_name} PRIVATE ${includes} ${GTEST_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/include")
    target_link_libraries(${test_name} PRIVATE ${libs} ${GTEST_LIBS})
    target_compile_features(${test_name} PUBLIC cxx_std_17)

    add_test(NAME ${test_name} COMMAND ${test_name})
    message(STATUS "Add ${test_name} to test")
endfunction()

#file(GLOB_RECURSE CoreSrcs "${CMAKE_SOURCE_DIR}/src/core/*.cpp")

add_test_project(Timer
    "${CMAKE_CURRENT_SOURCE_DIR}/core/util/timer/Timer.cxx"
    "${CoreSrcs}"
    ""
    "")
