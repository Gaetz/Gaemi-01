file( GLOB render_SOURCES *.cpp )
add_library( render ${render_SOURCES})
target_include_directories(render PUBLIC .)

target_link_libraries(render vma Vulkan::Vulkan)
