add_subdirectory(include)

add_subdirectory(src)

add_executable(Test1 ${TEST1_HEADERS} ${TEST1_SOURCES})

target_include_directories(Test1 PUBLIC ${TEST1_INCLUDE_DIRS})

target_link_libraries(Test1 PUBLIC TestBase RTLib)