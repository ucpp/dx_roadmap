project(engine)

set(ENGINE_HEADERS 
	# common
	include/common/pch.h 
	include/common/d3dx12.h 
	include/common/helpers.h 
	include/common/types.h 
	include/common/log.h 
	# core
	include/config.h
	include/window.h 
	include/application.h 
	include/device_resources.h 
)
set(ENGINE_SOURCES 
	# common
	sources/common/log.cpp 
	# core
	sources/config.cpp
	sources/window.cpp 
	sources/application.cpp 
	sources/device_resources.cpp 
)

add_library(${PROJECT_NAME} STATIC ${ENGINE_HEADERS} ${ENGINE_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC )
target_link_libraries(${PROJECT_NAME} PRIVATE d3d12.lib dxgi.lib dxguid.lib)
target_include_directories(${PROJECT_NAME} PUBLIC include ../externals/json/)