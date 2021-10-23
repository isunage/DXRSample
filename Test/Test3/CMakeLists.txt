add_subdirectory(include)

add_subdirectory(src)

add_executable(Test3 ${TEST3_HEADERS} ${TEST3_SOURCES})

set(TEST3_SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test3Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Test3Config.h)

target_include_directories(Test3 PUBLIC ${TEST3_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Test3 PUBLIC TestBase RTLib)

CopyDXC_DLL(Test3)