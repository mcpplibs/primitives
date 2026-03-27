set(PRIMITIVES_EXAMPLE_SOURCES
    ex01_basic_usage.cpp
    ex02_type_policy.cpp
    ex03_value_policy.cpp
    ex04_error_policy.cpp
    ex05_concurrency_policy.cpp
    ex06_conversion.cpp
    ex07_algorithms.cpp
    ex08_custom_underlying.cpp
    ex09_custom_policy.cpp
    ex10_custom_operation.cpp
)

foreach(example_source IN LISTS PRIMITIVES_EXAMPLE_SOURCES)
    get_filename_component(example_name ${example_source} NAME_WE)
    add_executable(${example_name} ${example_source})
    target_link_libraries(${example_name} PRIVATE mcpplibs-primitives)
endforeach()
