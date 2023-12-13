add_library(nvml_mod OBJECT nvml_entry.c hook.c)
target_compile_options(nvml_mod PUBLIC ${LIBRARY_COMPILE_FLAGS})
target_link_libraries(nvml_mod PUBLIC nvidia-ml -lcuda)
