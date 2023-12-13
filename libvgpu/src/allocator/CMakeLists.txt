add_library(allocator_mod OBJECT allocator.c)
target_compile_options(allocator_mod PUBLIC ${LIBRARY_COMPILE_FLAGS})
target_link_libraries(allocator_mod PUBLIC nvidia-ml)
