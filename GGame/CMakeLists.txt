## Game sources
file( GLOB GGame_SOURCES src/*.cpp )
add_library( GGame ${GGame_SOURCES} )
target_link_libraries(GGame GEngine SDL2)
target_include_directories(GGame PUBLIC GEngine)
#target_include_directories(GGame PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src") # useless, not a lib

## Executable
add_executable(${CMAKE_PROJECT_NAME} src/Main.cpp)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Gaemi>")
target_link_libraries(${CMAKE_PROJECT_NAME} GGame GEngine vkbootstrap vma glm imgui stb_image Vulkan::Vulkan SDL2 easy_profiler)

add_dependencies(${CMAKE_PROJECT_NAME} Shaders)

