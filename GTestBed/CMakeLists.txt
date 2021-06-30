## Executable
add_executable(GaemiTest src/Main.cpp)

set_property(TARGET GaemiTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Gaemi>")

target_include_directories(GaemiTest PUBLIC GEngine)
target_link_libraries(GaemiTest GEngine vkbootstrap vma glm imgui stb_image Vulkan::Vulkan SDL2)
# target_include_directories(GaemiTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src") # useless, not a lib


add_dependencies(GaemiTest Shaders)
