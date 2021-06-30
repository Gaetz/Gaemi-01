## Game sources
file( GLOB GGame_SOURCES src/*.cpp )
add_library( GGame ${GGame_SOURCES} )
target_include_directories(GGame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(GGame GEngine SDL2)

## Executable
add_executable(Gaemi src/Main.cpp)
set_property(TARGET Gaemi PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Gaemi>")
target_link_libraries(Gaemi GGame GEngine vkbootstrap vma glm imgui stb_image Vulkan::Vulkan SDL2)

add_dependencies(Gaemi Shaders)

