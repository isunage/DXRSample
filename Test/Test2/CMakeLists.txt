add_subdirectory(include)

add_subdirectory(src)

add_executable(Test2 ${TEST2_HEADERS} ${TEST2_SOURCES})

set(TEST2_SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test2Config.h.in ${CMAKE_CURRENT_BINARY_DIR}/Test2Config.h)

target_include_directories(Test2 PUBLIC ${TEST2_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Test2 PUBLIC TestBase RTLib dxcompiler.lib)

CopyDXC_DLL(Test2)