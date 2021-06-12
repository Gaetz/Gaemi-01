file( GLOB input_SOURCES *.cpp )
add_library( input ${input_SOURCES})
target_include_directories(input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(input SDL2)
