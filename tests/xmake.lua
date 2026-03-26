add_rules("mode.debug", "mode.release")

set_languages("c++23")

add_requires("gtest")

target("primitives_test")
    set_kind("binary")
    add_files("basic/main.cpp")
    add_files("basic/**/test_*.cpp")
    add_deps("mcpplibs-primitives")
    add_packages("gtest")
    add_links("gtest_main", "gmock_main")
    set_policy("build.c++.modules", true)
