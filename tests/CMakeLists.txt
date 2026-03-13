enable_testing()

if(NOT TARGET GTest::gtest)
	include(FetchContent)

	set(GTEST_VERSION_STABLE release-1.12.1 CACHE STRING "Stable GTest version")
	set(GTEST_VERSION_LATEST v1.16.0 CACHE STRING "Latest GTest version")

	if(NOT DEFINED GTEST_REPO)
		set(REPO_PRIORITY_LIST
				"git@github.com:google/googletest.git"
				"https://github.com/google/googletest.git"
				"https://gitee.com/mirrors/googletest.git"
		)

		foreach(repo IN LISTS REPO_PRIORITY_LIST)
			execute_process(
					COMMAND git ls-remote ${repo} -q
					RESULT_VARIABLE result
					OUTPUT_QUIET
					ERROR_QUIET
					TIMEOUT 5
			)
			if(result EQUAL 0)
				set(GTEST_REPO ${repo} CACHE STRING "Selected GTest repository")
				set(GTEST_VERSION ${GTEST_VERSION_LATEST} CACHE STRING "Selected GTest version")
				message(STATUS "Selected GTest repository: ${repo}")
				break()
			endif()
		endforeach()

		if(NOT GTEST_REPO)
			message(FATAL_ERROR "All GTest repositories are unavailable!")
		endif()
	endif()

	FetchContent_Declare(
			GTest
			GIT_REPOSITORY ${GTEST_REPO}
			GIT_TAG        ${GTEST_VERSION}
			GIT_SHALLOW    TRUE
	)

	FetchContent_MakeAvailable(GTest)
endif()

add_subdirectory(basic)
