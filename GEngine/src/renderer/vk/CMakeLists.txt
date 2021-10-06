file( GLOB vk_SOURCES *.cpp )
add_library( vk ${vk_SOURCES})
target_include_directories(vk PUBLIC .)

target_link_libraries(vk Vulkan::Vulkan vma tinyobjloader stb_image)
