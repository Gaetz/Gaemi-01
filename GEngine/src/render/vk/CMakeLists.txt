file( GLOB vk_SOURCES *.cpp )
add_library( vk ${vk_SOURCES})
target_include_directories(vk PUBLIC .)

target_link_libraries(vk imgui Vulkan::Vulkan vma tinyobjloader stb_image SDL2 math)
