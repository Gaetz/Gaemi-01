add_subdirectory( engine )
add_subdirectory( game )

## Executable
add_executable(Gaemi Main.cpp Log.cpp)

set_property(TARGET Gaemi PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Gaemi>")

target_include_directories(Gaemi PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Gaemi engine game vkbootstrap vma glm imgui stb_image Vulkan::Vulkan SDL2)

add_dependencies(Gaemi Shaders)
