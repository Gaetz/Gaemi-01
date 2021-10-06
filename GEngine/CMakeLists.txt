add_subdirectory( src/input )
add_subdirectory(src/renderer/vk)
add_subdirectory( src/math )
add_subdirectory( src/platforms )

add_definitions(-DGEXPORT)

file( GLOB GEngine_SOURCES src/*.cpp )
add_library( GEngine SHARED ${GEngine_SOURCES})
target_include_directories(GEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(GEngine input vk vma vkbootstrap math platforms Vulkan::Vulkan SDL2)
