cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
if (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

project(Gaemi VERSION 0.1.0)

## Vulkan
find_package(Vulkan REQUIRED)

## Debug definitions

## Debug fix for VsCode
if (CMAKE_BUILD_TYPE EQUAL "")
    # Build type is not set eg. command was "cmake .."
    message(STATUS "Build type was unspecified, set to Debug")
    set(CMAKE_BUILD_TYPE Debug)
else ()
    if (WIN32)
        set(CMAKE_BUILD_TYPE Debug)
    endif ()
    message(STATUS "  Build type specified as '${CMAKE_BUILD_TYPE}'")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "_DEBUG")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    add_definitions(-DBUILD_WITH_EASY_PROFILER=1)
    add_definitions(-DBUILD_WITH_OPTICK=1)
else ()
    set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "NDEBUG")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()

## Libs
add_subdirectory( externals )
add_subdirectory( GEngine )
add_subdirectory( GTestBed )
add_subdirectory( GGame )

## Vulkan validator
find_program(GLSL_VALIDATOR glslc HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## Find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp")

## Iterate each shader
message(STATUS "BUILDING SHADER")
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message("---- ${GLSL}")
  ##execute glslang command to compile that specific shader
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

## Copy shaders, assets and .dll/.so to build
set(BUILD_DIR cmake-build-debug)
set(BUILD_DIR_CLANG cmake-build-debug-clang)
if(WIN32)
  # Data
  file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
  file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
  # Dll
  if(EXISTS "${CMAKE_BINARY_DIR}/GEngine/GEngine.dll")
    file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed)
    file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame)
    #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed)
    #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame)
    file(COPY ${CMAKE_BINARY_DIR}/GEngine/GEngine.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed)
    file(COPY ${CMAKE_BINARY_DIR}/GEngine/GEngine.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame)
  elseif(EXISTS "${CMAKE_BINARY_DIR}/GEngine/Debug/GEngine.dll")
    file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed/Debug)
    file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame/Debug)
    #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed/Debug)
    #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame/Debug)
    file(COPY ${CMAKE_BINARY_DIR}/GEngine/Debug/GEngine.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed/Debug)
    file(COPY ${CMAKE_BINARY_DIR}/GEngine/Debug/GEngine.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame/Debug)
  else()
    #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed)
    #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame)
    file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GTestBed)
    file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR}/GGame)
    # CLion
    if(EXISTS "${BUILD_DIR}/GEngine/GEngine.dll")
        file(COPY ${BUILD_DIR}/GEngine/GEngine.dll DESTINATION ${BUILD_DIR}/GTestBed)
        file(COPY ${BUILD_DIR}/GEngine/GEngine.dll DESTINATION ${BUILD_DIR}/GGame)
        file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${BUILD_DIR}/GTestBed)
        file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${BUILD_DIR}/GGame)
        #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${BUILD_DIR}/GTestBed)
        #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${BUILD_DIR}/GGame)
    endif()
    if(EXISTS "${BUILD_DIR_CLANG}/GEngine/GEngine.dll")
        file(COPY ${BUILD_DIR_CLANG}/GEngine/GEngine.dll DESTINATION ${BUILD_DIR_CLANG}/GTestBed)
        file(COPY ${BUILD_DIR_CLANG}/GEngine/GEngine.dll DESTINATION ${BUILD_DIR_CLANG}/GGame)
        file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${BUILD_DIR_CLANG}/GTestBed)
        file(COPY externals/SDL2-2.0.14/lib/x64/SDL2.dll DESTINATION ${BUILD_DIR_CLANG}/GGame)
        #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${BUILD_DIR_CLANG}/GTestBed)
        #file(COPY externals/easy_profiler/bin/easy_profiler.dll DESTINATION ${BUILD_DIR_CLANG}/GGame)
    endif()
  endif()
elseif(UNIX)
    file(COPY ${BUILD_DIR}/GEngine/libGEngine.so DESTINATION ${BUILD_DIR}/GTestBed)
    file(COPY ${BUILD_DIR}/GEngine/libGEngine.so DESTINATION ${BUILD_DIR}/GGame)
    file(COPY ${BUILD_DIR_CLANG}/GEngine/libGEngine.so DESTINATION ${BUILD_DIR_CLANG}/GTestBed)
    file(COPY ${BUILD_DIR_CLANG}/GEngine/libGEngine.so DESTINATION ${BUILD_DIR_CLANG}/GGame)
endif()
