add_library(cuda_mod OBJECT context.c device.c hook.c event.c hook.c memory.c stream.c graph.c)
target_compile_options(cuda_mod PUBLIC ${LIBRARY_COMPILE_FLAGS})
target_link_libraries(cuda_mod PUBLIC nvidia-ml -lcuda)
