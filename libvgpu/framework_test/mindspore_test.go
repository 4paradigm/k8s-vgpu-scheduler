package vgpu_test

import (
	"context"
	"fmt"

	"github.com/docker/docker/api/types/mount"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	. "gitlab.4pd.io/libvgpu/framework_test/clients"
	. "gitlab.4pd.io/libvgpu/framework_test/config"
)

func distributedTrainingTest(NVIDIA_DEVICE_MAP string) {
	ctx := context.Background()
	img := fmt.Sprintf("%s/mindspore_multi_gpu:v1.1.1", GetDockerRegistry())
	cmd := []string{"sh", "-c", "(cd /root/docs/tutorials/tutorial_code/distributed_training && bash run_gpu.sh /root/docs/tutorials/tutorial_code/distributed_training/cifar-10-batches-bin) 2>&1"}
	env := []string{
		"NVIDIA_VISIBLE_DEVICES=0",
		fmt.Sprintf("NVIDIA_DEVICE_MAP=%s", NVIDIA_DEVICE_MAP),
		"LD_PRELOAD=/usr/local/vgpu/libvgpu_2.so",
		"CUDA_OVERSUBSCRIBE=true",
	}
	mount := []mount.Mount{
		{
			Type:   mount.TypeBind,
			Source: "/home/gitlab-runner/vgpu",
			Target: "/usr/local/vgpu",
		},
		{
			Type:   mount.TypeBind,
			Source: GetExamplesPath(),
			Target: "/examples",
		},
	}
	log := RunContainer(ctx, img, cmd, env, mount, GetShmSize())
	fmt.Printf("log: %s\n", log)
	Expect(log).Should(ContainSubstring("Done"))
}

var _ = Describe("mindspore", func() {
	Describe("Mindspore", func() {
		Context("when 1虚2", func() {
			It("log should contain keyword", func() {
				distributedTrainingTest(vgpu1Map2)
			})
		})
		Context("when 1虚3", func() {
			It("log should contain keyword", func() {
				distributedTrainingTest(vgpu1Map3)
			})
		})
		Context("when 1虚4", func() {
			It("log should contain keyword", func() {
				distributedTrainingTest(vgpu1Map4)
			})
		})
	})
})
