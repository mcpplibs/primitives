add_rules("mode.debug", "mode.release")

set_languages("c++23")

add_requires("gtest")

target("primitives_test")
    set_kind("binary")
    add_files("basic/*.cpp")
    add_deps("mcpplibs-primitives")
    add_packages("gtest")
    set_policy("build.c++.modules", true)
