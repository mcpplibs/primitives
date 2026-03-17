add_rules("mode.debug", "mode.release")

set_languages("c++23")

local examples = {
    "ex01_default_arithmetic",
    "ex02_type_policy",
    "ex03_value_policy",
    "ex04_error_policy",
    "ex05_concurrency_policy",
    "ex06_custom_underlying",
    "ex07_custom_policy",
    "ex08_custom_operation"
}

for _, name in ipairs(examples) do
    target(name)
        set_kind("binary")
        add_files(name .. ".cpp")
        add_deps("mcpplibs-primitives")
        set_policy("build.c++.modules", true)
end
