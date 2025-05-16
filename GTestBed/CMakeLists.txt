## Test bed sources
file( GLOB GTestBed_SOURCES src/*.cpp )
add_library( GTestBed ${GTestBed_SOURCES} )
target_link_libraries(GTestBed GEngine SDL2)
target_include_directories(GTestBed PUBLIC GEngine)
#target_include_directories(GTestBest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src") # useless, not a lib



## Executable
add_executable(GaemiTest src/Main.cpp)
set_property(TARGET GaemiTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Gaemi>")
target_link_libraries(GaemiTest GTestBed GEngine vkbootstrap vma glm imgui stb_image Vulkan::Vulkan SDL2) # easy_profiler)
# target_include_directories(GaemiTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src") # useless, not a lib

add_dependencies(GaemiTest Shaders)
