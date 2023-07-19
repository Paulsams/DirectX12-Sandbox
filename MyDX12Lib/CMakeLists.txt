cmake_minimum_required( VERSION 3.18.3 )

set( TARGET_NAME MyDX12Lib )

set(CMAKE_CXX_STANDARD 20)

set( HEADER_FILES
    inc/Application.h
	inc/Camera.h
    inc/CommandQueue.h
    inc/d3dx12.h
	inc/Defines.h
	inc/DescriptorAllocation.h
	inc/DescriptorAllocator.h
	inc/DescriptorAllocatorPage.h
	inc/Defines.h
	inc/Defines.h
    inc/DX12LibPCH.h
    inc/DynamicDescriptorHeap.h
	inc/Events.h
    inc/Game.h
    inc/Helpers.h
    inc/HighResolutionClock.h
	inc/Mathf.h
    inc/Resource.h
	inc/ResourceStateTracker.h
	inc/RootSignature.h
	inc/TextureUsage.h
	inc/UploadBuffer.h
    inc/Window.h
	resource.h
)

set( SOURCE_FILES
    src/Application.cpp
	src/Camera.cpp
    src/CommandQueue.cpp
    src/DescriptorAllocation.cpp
    src/DescriptorAllocator.cpp
    src/DescriptorAllocatorPage.cpp
    src/DX12LibPCH.cpp
    src/DynamicDescriptorHeap.cpp
    src/Game.cpp
    src/HighResolutionClock.cpp
    src/Resource.cpp
    src/ResourceStateTracker.cpp
    src/RootSignature.cpp
    src/UploadBuffer.cpp
    src/Window.cpp
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
