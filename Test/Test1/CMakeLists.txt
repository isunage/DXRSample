add_subdirectory(include)

add_subdirectory(src)

add_executable(Test1 ${TEST1_HEADERS} ${TEST1_SOURCES})

set(TEST1_SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test1Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Test1Config.h)

target_include_directories(Test1 PUBLIC ${TEST1_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Test1 PUBLIC TestBase RTLib dxcompiler.lib)

CopyDXC_DLL(Test1)