set(DXC_INCLUDE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(DXC_LIBRARIES    ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/dxcompiler.lib)

add_library(dxc INTERFACE)
target_include_directories(dxc INTERFACE ${DXC_INCLUDE_DIR})
target_link_libraries(dxc INTERFACE      ${DXC_LIBRARIES})

set(DXC_DYNAMIC_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/x64 CACHE PATH "DXC DLL DIRECTOY")
function(CopyDXC_DLL Target)
    set(DXC_DYNAMIC_LIBRARIES ${DXC_DYNAMIC_LIBRARY_PATH}/dxcompiler.dll ${DXC_DYNAMIC_LIBRARY_PATH}/dxil.dll)
    foreach(dll ${DXC_DYNAMIC_LIBRARIES})
        add_custom_command(TARGET ${Target} POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy "${dll}" "$<TARGET_FILE_DIR:${Target}>"
        )
    endforeach(dll)
endfunction()