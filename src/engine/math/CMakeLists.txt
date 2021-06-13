file( GLOB math_SOURCES *.cpp )
add_library( math ${math_SOURCES})
target_include_directories(math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(math glm)
