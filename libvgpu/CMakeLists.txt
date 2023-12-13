cmake_minimum_required (VERSION 2.8.11)
project (libvgpu)

macro(get_git_hash _git_hash)   
    find_package(Git QUIET)     
    if(GIT_FOUND)
      execute_process(          
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h 
        OUTPUT_VARIABLE ${_git_hash}        
        OUTPUT_STRIP_TRAILING_WHITESPACE    
        ERROR_QUIET                         
        WORKING_DIRECTORY                   
          ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endmacro() 

macro(get_git_branch _git_branch)
    find_package(Git QUIET)
    if(GIT_FOUND)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
        OUTPUT_VARIABLE ${_git_branch}
        OUTPUT_STRIP_TRAILING_WHITESPACE
      #  ERROR_QUIET
        WORKING_DIRECTORY
          ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endmacro()

set(GIT_HASH "")
get_git_hash(GIT_HASH)
message(STATUS "Git hash is ${GIT_HASH}")

if (NOT DEFINED ENV{CI_COMMIT_REF_NAME})
  set(GIT_BRANCH "")
  get_git_branch(GIT_BRANCH)
else()
  set(GIT_BRANCH $ENV{CI_COMMIT_REF_NAME})
endif()

if(GIT_FOUND)
  #string(REPLACE "." "_" GIT_BRANCH ${GIT_BRANCH})
  #string(REPLACE "-" "_" GIT_BRANCH ${GIT_BRANCH})
  #message(STATUS "Git branch is ${GIT_BRANCH}")
endif()

configure_file(src/static_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config/static_config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/config)

if (NOT DEFINED ENV{CUDA_HOME})
    set(CUDA_HOME /usr/local/cuda)
else()
    set(CUDA_HOME $ENV{CUDA_HOME})
endif()


if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(LIBRARY_COMPILE_FLAGS -shared -fPIC -g -D_GNU_SOURCE -Wall)
    set(TEST_COMPILE_FLAGS -o1)
else()
    set(LIBRARY_COMPILE_FLAGS -shared -fPIC -D_GNU_SOURCE -fvisibility=hidden -Wall)
    set(TEST_COMPILE_FLAGS -o1)
endif()

include_directories(${CUDA_HOME}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)
add_subdirectory(test)

if (BUILD_DOCKER STREQUAL true)
	add_subdirectory(dockerfiles)
endif()

