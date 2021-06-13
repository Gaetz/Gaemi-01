find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)

add_library(stb_image INTERFACE)

add_library(tinyobjloader STATIC)

target_sources(vkbootstrap PRIVATE
    vkbootstrap/VkBootstrap.h
    vkbootstrap/VkBootstrap.cpp
    )

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

#both vma and glm and header only libs so we only need the include path
target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)

target_sources(tinyobjloader PRIVATE
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

## SDL2
add_library(SDL2 INTERFACE)

if (WIN32)
  set(SDL2_DIR "${CMAKE_SOURCE_DIR}/externals/SDL2-2.0.14")
  set(SDL_PATH "${SDL2_DIR}")
  find_package(SDL2 REQUIRED)
  target_include_directories(SDL2 INTERFACE ${SDL2_DIR}/include)
  target_link_directories(SDL2 INTERFACE ${SDL2_DIR}/lib/x64)
elseif (UNIX)
  find_package(SDL2 REQUIRED)
  target_include_directories(SDL2 INTERFACE ${SDL2_DIR}/include)
  target_link_directories(SDL2 INTERFACE ${SDL2_DIR}/lib)
endif ()

target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARIES})

## imgui
add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp

    imgui/imgui_impl_vulkan.cpp
    imgui/imgui_impl_sdl.cpp
    )

target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2)

target_include_directories(stb_image INTERFACE stb_image)
