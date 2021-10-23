add_subdirectory(include)

add_subdirectory(src)

add_executable(Test4 ${TEST4_HEADERS} ${TEST4_SOURCES})

set(TEST4_SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test4Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Test4Config.h)

target_include_directories(Test4 PUBLIC ${TEST4_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Test4 PUBLIC TestBase RTLib)

CopyDXC_DLL(Test4)
