add_subdirectory( engine )
add_subdirectory( game )

## Executable
add_executable(Gaemi Main.cpp)

set_property(TARGET Gaemi PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Gaemi>")

#target_link_libraries(Gaemi game engine ${SDL2_LIBRARIES})
#target_link_libraries(Gaemi game engine)

target_include_directories(Gaemi PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Gaemi engine game vkbootstrap vma glm tinyobjloader imgui stb_image Vulkan::Vulkan SDL2)

add_dependencies(Gaemi Shaders)