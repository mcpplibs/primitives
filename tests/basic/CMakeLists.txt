# tests/basic/CMakeLists.txt

enable_testing()

file(GLOB BASIC_TESTS "test_*.cpp")

add_executable(basic_tests ${BASIC_TESTS})

target_link_libraries(basic_tests PRIVATE GTest::gtest GTest::gmock GTest::gmock_main GTest::gtest_main)
target_compile_features(basic_tests PRIVATE cxx_std_23)

target_link_libraries(basic_tests PRIVATE mcpplibs-primitives)

add_test(
        NAME BASIC_TESTS
        COMMAND $<TARGET_FILE:basic_tests>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
