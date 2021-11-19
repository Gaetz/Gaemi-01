find_package(Vulkan REQUIRED)

message("-- Integrating external libraries")

# Both vma, glm and sbt_image are header only libs so we only need the include path

## GLM
message("---- Linking GLM")
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

## Vulkan Memory Allocator
message("---- Linking VMA")
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)

## SBT image
message("---- Linking SBT Image")
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb_image)

## vkBootstrap
message("---- Linking vkBootstrap")
add_library(vkbootstrap STATIC)
target_sources(vkbootstrap PRIVATE
    vkbootstrap/VkBootstrap.h
    vkbootstrap/VkBootstrap.cpp
    )
target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

## tinyobjloader
message("---- Linking tinyobjloader")
add_library(tinyobjloader STATIC)
target_sources(tinyobjloader PRIVATE
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )
target_include_directories(tinyobjloader PUBLIC tinyobjloader)

## SDL2
message("---- Linking SDL2")
add_library(SDL2 INTERFACE)

if (WIN32)
  set(SDL2_DIR "${CMAKE_SOURCE_DIR}/externals/SDL2-2.0.14")
  set(SDL_PATH "${SDL2_DIR}")
  find_package(SDL2 REQUIRED)
  target_include_directories(SDL2 INTERFACE ${SDL2_DIR}/include)
  target_link_directories(SDL2 INTERFACE ${SDL2_LIB_PATH})
elseif (UNIX)
  find_package(SDL2 REQUIRED)
  target_include_directories(SDL2 INTERFACE ${SDL2_DIR}/include)
  target_link_directories(SDL2 INTERFACE ${SDL2_DIR}/lib)
endif ()

target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARIES})

## imgui
message("---- Linking ImGUI")
add_library(imgui STATIC)
target_sources(imgui PRIVATE
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp

    imgui/imgui_impl_vulkan.cpp
    imgui/imgui_impl_sdl.cpp
    )
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2)

## Easy profiler
message("---- Linking easy_profiler")
set(easy_profiler_DIR "${CMAKE_SOURCE_DIR}/externals/easy_profiler/lib/cmake/easy_profiler")
set(EASY_PROFILER_NO_GUI ON CACHE BOOL "")
set(EASY_PROFILER_NO_SAMPLES ON CACHE BOOL "")
add_library(easy_profiler easy_profiler/bin/easy_profiler.dll)
find_package(easy_profiler REQUIRED)
set_target_properties(easy_profiler PROPERTIES LINKER_LANGUAGE C)
file(COPY "${CMAKE_SOURCE_DIR}/externals/easy_profiler/lib/easy_profiler.lib" DESTINATION ${CMAKE_BINARY_DIR}/externals)

## Assimp
message("---- Linking Assimp ")
add_library(assimp assimp/assimp-vc142-mtd.dll)
target_include_directories(assimp PUBLIC assimp/include)
target_link_directories(assimp PUBLIC assimp/lib)
set_target_properties(assimp PROPERTIES LINKER_LANGUAGE C)
file(COPY "${CMAKE_SOURCE_DIR}/externals/assimp/lib/assimp.lib" DESTINATION ${CMAKE_BINARY_DIR}/externals)

## Etc2Comp
message("---- Linking ETC2Comp")
add_subdirectory(etc2comp)
set(EtcLib_DIR "${CMAKE_SOURCE_DIR}/externals/etc2comp")
find_package(EtcLib REQUIRED)
set(BUILD_DIR_CLANG "${CMAKE_SOURCE_DIR}/cmake-build-debug-clang")
target_include_directories(EtcLib PUBLIC ${ETCLIB_INCLUDE_DIRS})
target_link_directories(EtcLib PUBLIC ${ETCLIB_LIBRARIES})
