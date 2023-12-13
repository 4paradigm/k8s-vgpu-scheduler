#!/bin/sh

# make linux build and upload to env 77.
#CGO_ENABLED=0 GOOS=linux ginkgo build .
#scp -r vgpu_test.test work@m5-cloudinfra-online01:/home/work/chenxiangwen/test

# local test
rm ./framework_test.test
go mod tidy
ginkgo build .
./framework_test.test  -ginkgo.focus "1虚2"  -ginkgo.skip mindspore

# use "focus" to run specific cases
# ./vgpu_test.test -ginkgo.progress -ginkgo.focus tensorflow

