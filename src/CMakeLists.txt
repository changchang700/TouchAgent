add_compile_options(-O3)
add_executable(TouchAgent touch.cc)
target_link_libraries(TouchAgent local_hv)
target_link_libraries(TouchAgent log)
