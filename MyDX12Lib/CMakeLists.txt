cmake_minimum_required( VERSION 3.18.3 )

set( TARGET_NAME MyDX12Lib )

set(CMAKE_CXX_STANDARD 20)

set( HEADER_FILES
    inc/DX12LibPCH.h
    inc/Application.h
    inc/CommandQueue.h
    inc/Game.h
    inc/Helpers.h
	inc/Events.h
    inc/HighResolutionClock.h
	resource.h
    inc/Window.h
    inc/d3dx12.h
	inc/Mathf.h
	inc/Camera.h
)

set( SOURCE_FILES
    src/DX12LibPCH.cpp
    src/Application.cpp
    src/CommandQueue.cpp
    src/Game.cpp
    src/HighResolutionClock.cpp
    src/Window.cpp
	src/Camera.cpp
)

set( RESOURCE_FILES 
    DX12Lib.rc
    Resources/Icon/app_icon.ico
)

source_group( "Resources" FILES DX12Lib.rc )
source_group( "Resources\\Icon" FILES Resources/Icon/app_icon.ico )

add_library( ${TARGET_NAME} STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
)

target_include_directories( ${TARGET_NAME}
    PUBLIC inc
)

target_link_libraries( ${TARGET_NAME}
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    # PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.dir/${CMAKE_CFG_INTDIR}/${TARGET_NAME}.res # This is the only way I could figure out how to link the compiled resource file.
)
