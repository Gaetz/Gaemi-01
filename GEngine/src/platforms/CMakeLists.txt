file( GLOB platforms_SOURCES *.cpp )
add_library( platforms ${platforms_SOURCES})
target_include_directories(platforms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(platforms Vulkan::Vulkan vma SDL2)
