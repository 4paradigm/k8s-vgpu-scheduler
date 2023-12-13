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

func pytorchTest(img string, NVIDIA_DEVICE_MAP string) {
	ctx := context.Background()
	cmd := []string{"sh", "-c", "pip install tensorboard && cd /examples && python data_parallel_train.py 2>&1"}
	env := []string{
		GetNvidiaEnv(NVIDIA_DEVICE_MAP),
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
	Expect(log).Should(ContainSubstring("loss:"))
}

var _ = Describe("pytorch", func() {
	Describe("pytorch 1.8.0-cuda11.1-cudnn8-runtime", func() {
		Context("1虚2", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.8.0-cuda11.1-cudnn8-runtime", vgpu1Map2)
			})
		})
		Context("1虚2_1", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.8.0-cuda11.1-cudnn8-runtime", vgpu1Map21)
			})
		})
		Context("1虚2_2", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.8.0-cuda11.1-cudnn8-runtime", vgpu1Map22)
			})
		})
		Context("1虚3", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.8.0-cuda11.1-cudnn8-runtime", vgpu1Map3)
			})
		})
		Context("1虚4", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.8.0-cuda11.1-cudnn8-runtime", vgpu1Map4)
			})
		})
	})
	Describe("pytorch 1.4-cuda10.1-cudnn7-runtime", func() {
		Context("1虚2", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.4-cuda10.1-cudnn7-runtime", vgpu1Map2)
			})
		})
		Context("1虚3", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.4-cuda10.1-cudnn7-runtime", vgpu1Map3)
			})
		})
		Context("1虚4", func() {
			It("log should contain keyword", func() {
				pytorchTest("m7-ieg-pico-test01:5000/pytorch/pytorch-flask:1.4-cuda10.1-cudnn7-runtime", vgpu1Map4)
			})
		})
	})
})
