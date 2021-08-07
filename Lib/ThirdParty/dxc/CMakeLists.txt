set(DXC_DYNAMIC_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin CACHE PATH "DXC DLL DIRECTOY")

function(CopyDXC_DLL Target)
    set(DXC_DYNAMIC_LIBRARIES ${DXC_DYNAMIC_LIBRARY_PATH}/dxcompiler.dll ${DXC_DYNAMIC_LIBRARY_PATH}/dxil.dll)
    foreach(dll ${DXC_DYNAMIC_LIBRARIES})
        add_custom_command(TARGET ${Target} POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy "${dll}" "$<TARGET_FILE_DIR:${Target}>"
        )
    endforeach(dll)
endfunction()