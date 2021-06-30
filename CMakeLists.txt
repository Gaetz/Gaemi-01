cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

project(Gaemi VERSION 0.1.0)

## Vulkan
find_package(Vulkan REQUIRED)


## Debug fix for VsCode
#if (WIN32)
    if (CMAKE_BUILD_TYPE MATCHES "")
        # Build type is not set eg. command was "cmake .."
        message(STATUS "  Build type was unspecified, set to Debug")
        set(CMAKE_BUILD_TYPE Debug)
        add_definitions(-D_DEBUG)
    else ()
        message(STATUS "  Build type specified as '${CMAKE_BUILD_TYPE}'")
    endif ()
#endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "_DEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
else ()
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "NDEBUG")
endif ()

## Libs
add_subdirectory( externals )
add_subdirectory( GEngine )
add_subdirectory( GTestBed )
add_subdirectory( GGame )

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")



## Vulkan validator
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## Find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp")

## Iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  ##execute glslang command to compile that specific shader
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

##Copy shaders and assets to build under Windows
if(WIN32)
  file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
  file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
  file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed)
  file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame)
endif()