add_subdirectory(include)

add_subdirectory(src)

add_library(RTLib STATIC ${RTLIB_SOURCES} ${RTLIB_HEADERS})

target_include_directories(RTLib PUBLIC ${RTLIB_INCLUDE_DIRS})

target_link_libraries(RTLib PUBLIC d3d12.lib dxgi.lib d3dcompiler.lib dxguid.lib d3dx12 fmt)

