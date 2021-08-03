add_subdirectory(include)

add_subdirectory(src)

add_executable(Test0 ${TEST0_SOURCES})
target_link_libraries(Test0 PUBLIC TestBase)