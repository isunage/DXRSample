add_subdirectory(include)

add_subdirectory(src)

add_executable(Test0 ${TEST0_HEADERS} ${TEST0_SOURCES})

target_include_directories(Test0 PUBLIC ${TEST0_INCLUDE_DIRS})

target_link_libraries(Test0 PUBLIC TestBase RTLib)