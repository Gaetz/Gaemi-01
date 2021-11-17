add_subdirectory( src/input )
add_subdirectory( src/mem )
add_subdirectory( src/render )
add_subdirectory( src/render/vk )
add_subdirectory( src/math )
add_subdirectory( src/platforms )
add_subdirectory( src/asset )

add_definitions(-DGEXPORT)

file( GLOB GEngine_SOURCES src/*.cpp )
add_library( GEngine SHARED ${GEngine_SOURCES})
target_include_directories(GEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(GEngine input mem render vma vkbootstrap math platforms asset Vulkan::Vulkan SDL2 easy_profiler assimp)
