#!/bin/bash
root_dir=$(cd $(dirname $0); pwd)
cd $root_dir

mkdir -p build; cd build

[[ -z "$J" ]] && J=`nproc | awk '{print int(($0 + 1)/ 2)}'`
CMAKE=${CMAKE-"cmake3"}
CMAKE_OPTIONS=${CMAKE_OPTIONS-""}
#CMAKE_OPTIONS+=" -DMEMORY_LIMIT_DEBUG=1"
CMAKE_OPTIONS+=" -DDLSYM_HOOK_ENABLE=1"
#CMAKE_OPTIONS+=" -DDLSYM_HOOK_DEBUG=1"
CMAKE_OPTIONS+=" -DMULTIPROCESS_LIMIT_ENABLE=1"
CMAKE_OPTIONS+=" -DHOOK_MEMINFO_ENABLE=1"
CMAKE_OPTIONS+=" -DHOOK_NVML_ENABLE=1"
CMAKE_OPTIONS+=" -DCMAKE_BUILD_TYPE=Debug"

#CMAKE_OPTIONS+=" -DBUILD_DOCKER=true"
# gitlab ci
CI_COMMIT_BRANCH=${CI_COMMIT_BRANCH-""}
CI_COMMIT_SHA=${CI_COMMIT_SHA-""}

# jenkins
if [ -z $CI_COMMIT_BRANCH ]; then
    CI_COMMIT_BRANCH=${BRANCH_NAME-"unknown"}
fi
if [ -z $CI_COMMIT_SHA ]; then
    CI_COMMIT_SHA=$(git describe --abbrev=100 --always)
    if [ $? -ne 0 ]; then
        CI_COMMIT_SHA="unknown"
    fi
fi

echo "CI_COMMIT_BRANCH:${CI_COMMIT_BRANCH}"
echo "CI_COMMIT_SHA:${CI_COMMIT_SHA}"

${CMAKE} .. ${CMAKE_OPTIONS} -DTEST_DEVICE_ID=0
make -j$J
