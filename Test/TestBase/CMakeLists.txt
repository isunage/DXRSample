add_subdirectory(include)
add_subdirectory(src)
add_library(TestBase STATIC ${TEST_BASE_HEADERS} ${TEST_BASE_SOURCES})
target_include_directories(TestBase PUBLIC ${TEST_BASE_INCLUDE_DIRS})