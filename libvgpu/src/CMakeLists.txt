cmake_minimum_required (VERSION 2.8.11)
project (libvgpu)


# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CUDA_FILES libcuda_hook.c libcuda_context.c libcuda_device.c libcuda_stream.c libcuda_memory.c)

add_subdirectory(multiprocess)
add_subdirectory(allocator)
add_subdirectory(cuda)
add_subdirectory(nvml)

set(LIBVGPU vgpu)
add_library(${LIBVGPU} SHARED libvgpu.c utils.c $<TARGET_OBJECTS:nvml_mod> $<TARGET_OBJECTS:cuda_mod> $<TARGET_OBJECTS:allocator_mod> $<TARGET_OBJECTS:multiprocess_mod>)
target_compile_options(${LIBVGPU} PUBLIC ${LIBRARY_COMPILE_FLAGS})
target_link_libraries(${LIBVGPU} PUBLIC -lcuda -lnvidia-ml)

add_custom_target(strip_symbol ALL
    COMMAND strip -x ${CMAKE_BINARY_DIR}/lib${LIBVGPU}.so
    DEPENDS ${LIBVGPU})
