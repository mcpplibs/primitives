enable_testing()

function(add_basic_suite target_name suite_pattern)
    file(GLOB_RECURSE suite_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${suite_pattern}"
    )

    if(NOT suite_sources)
        return()
    endif()

    add_executable(${target_name} ${suite_sources})
    target_compile_features(${target_name} PRIVATE cxx_std_23)
    target_link_libraries(${target_name}
        PRIVATE
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            GTest::gtest_main
            mcpplibs-primitives
    )

    add_test(
        NAME ${target_name}
        COMMAND $<TARGET_FILE:${target_name}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

add_basic_suite(basic_conversion_tests "conversion/**/test_*.cpp")
add_basic_suite(basic_algorithms_tests "algorithms/**/test_*.cpp")
add_basic_suite(basic_literals_tests "literals/test_*.cpp")
add_basic_suite(basic_operations_tests "operations/**/test_*.cpp")
add_basic_suite(basic_policy_tests "policy/**/test_*.cpp")
add_basic_suite(basic_primitive_tests "primitive/**/test_*.cpp")
add_basic_suite(basic_underlying_tests "underlying/**/test_*.cpp")

add_custom_target(basic_tests)
add_dependencies(basic_tests
    basic_conversion_tests
    basic_algorithms_tests
    basic_literals_tests
    basic_operations_tests
    basic_policy_tests
    basic_primitive_tests
    basic_underlying_tests
)
