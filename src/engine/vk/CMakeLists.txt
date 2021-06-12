file( GLOB vk_SOURCES *.cpp )
add_library( vk ${vk_SOURCES})
target_include_directories(vk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vk Vulkan::Vulkan SDL2)
