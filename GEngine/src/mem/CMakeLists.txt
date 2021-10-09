file( GLOB mem_SOURCES *.cpp )
add_library( mem ${mem_SOURCES})
target_include_directories(mem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mem)
