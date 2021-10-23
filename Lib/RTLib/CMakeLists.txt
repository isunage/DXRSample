add_subdirectory(include)

add_subdirectory(src)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/RTLibConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/RTLibConfig.h)

set(RTLIB_INCLUDE_DIRS ${RTLIB_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

add_library(RTLib STATIC ${RTLIB_SOURCES} ${RTLIB_HEADERS})

target_include_directories(RTLib PUBLIC ${RTLIB_INCLUDE_DIRS})

target_link_libraries(RTLib PUBLIC d3d12.lib dxgi.lib d3dcompiler.lib dxguid.lib d3dx12 fmt dxc tinyobjloader)


