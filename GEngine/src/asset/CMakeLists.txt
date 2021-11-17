file( GLOB asset_SOURCES *.cpp )
add_library( asset ${asset_SOURCES})
target_include_directories(asset PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(asset vk vma Vulkan::Vulkan assimp)
