cmake_minimum_required(VERSION 3.16)
project(vtk_renderer)

set(CMAKE_CXX_STANDARD 11)

if(APPLE)
    set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX10.14.sdk")
endif()

add_compile_definitions(NON_MATLAB_PARSING)
add_compile_definitions(MAX_EXT_API_CONNECTIONS=255)
add_compile_definitions(DO_NOT_USE_SHARED_MEMORY)

include_directories(${COPPELIA_INCLUDE_DIR})

find_package(VTK REQUIRED)
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
include(${VTK_USE_FILE})

add_executable(vtk_renderer src/main.cpp
        src/vtk_renderer/timerClass.h src/vtk_renderer/timerClass.cpp
        src/vtk_renderer/pathObject.h src/vtk_renderer/pathObject.cpp
        src/vtk_renderer/vrep_volume_grid.h src/vtk_renderer/vrep_volume_grid.cpp
        src/vtk_renderer/vrep_light.h src/vtk_renderer/vrep_light.cpp
        src/vtk_renderer/vrep_vision_sensor.h src/vtk_renderer/vrep_vision_sensor.cpp
        src/vtk_renderer/vrep_mesh_reader.h src/vtk_renderer/vrep_mesh_reader.cpp
        src/vtk_renderer/vrep_mesh_object.h src/vtk_renderer/vrep_mesh_object.cpp
        src/vtk_renderer/vrep_scene_content.h src/vtk_renderer/vrep_scene_content.cpp
        src/vtk_renderer/renderwindow_support.h src/vtk_renderer/renderwindow_support.cpp
        src/remoteAPI/extApi.c src/remoteAPI/extApi.h
        src/remoteAPI/extApiPlatform.c src/remoteAPI/extApiPlatform.h
        )

target_link_libraries(vtk_renderer PRIVATE ${VTK_LIBRARIES})