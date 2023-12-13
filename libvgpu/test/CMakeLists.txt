
find_package(CUDA REQUIRED)

set(TEST_CPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_TARGET_NAMES_LIST) 


file(GLOB_RECURSE TEST_SCRIPTS "${TEST_CPP_SOURCE_DIR}/*.c" "${TEST_CPP_SOURCE_DIR}/*.cu")
foreach(TEST_SCRIPT ${TEST_SCRIPTS}) 
    file(RELATIVE_PATH RELATIVE_TEST_PATH ${TEST_CPP_SOURCE_DIR} ${TEST_SCRIPT})
    get_filename_component(TEST_TARGET_DIR ${RELATIVE_TEST_PATH} DIRECTORY)
    get_filename_component(TEST_TARGET_NAME ${RELATIVE_TEST_PATH} NAME_WE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TEST_TARGET_DIR})

    if (TEST_SCRIPT MATCHES ".*cu")
        cuda_add_executable(${TEST_TARGET_NAME}  ${TEST_SCRIPT})
    else()
        add_executable(${TEST_TARGET_NAME}  ${TEST_SCRIPT})
    endif()

    list(APPEND TEST_TARGET_NAMES_LIST ${TEST_TARGET_NAME})
    target_link_libraries(${TEST_TARGET_NAME}  -lrt -lpthread -lnvidia-ml -lcuda -lcudart -L${CUDA_HOME}/lib64)
    target_compile_options(${TEST_TARGET_NAME} PUBLIC ${TEST_COMPILE_FLAGS})

    if (DEFINED TEST_DEVICE_ID)
    	target_compile_definitions(${TEST_TARGET_NAME} PUBLIC -DTEST_DEVICE_ID=${TEST_DEVICE_ID})
	endif()
endforeach()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


add_custom_target(python_test ALL
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/python ${CMAKE_CURRENT_BINARY_DIR})

# add_custom_target(multiprocess_test_scripts ALL
#    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/multiprocess/scripts
#                  ${CMAKE_CURRENT_BINARY_DIR}/multiprocess)