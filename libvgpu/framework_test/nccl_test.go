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

func ncclTest(img string, NVIDIA_DEVICE_MAP string, vir string) {
	ctx := context.Background()
	cmd := []string{"/root/nccl-tests-2.6.4/build/all_reduce_perf", "-b", "8", "-e", "128M", "-f", "2", "-g", vir}
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
	}
	log := RunContainer(ctx, img, cmd, env, mount, GetShmSize())
	fmt.Printf("log: %s\n", log)
	Expect(log).Should(ContainSubstring("OK"))
}

var _ = Describe("nccl", func() {
	Describe("nccl_2.6.8test", func() {
		Context("when 1虚2", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.6.8-cuda10", vgpu1Map2, "2")
			})
		})
		Context("when 1虚2_1", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.6.8-cuda10", vgpu1Map21, "2")
			})
		})
		Context("when 1虚2_2", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.6.8-cuda10", vgpu1Map22, "2")
			})
		})
		Context("when 1虚3", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.6.8-cuda10", vgpu1Map3, "3")
			})
		})
		Context("when 1虚4", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.6.8-cuda10", vgpu1Map4, "4")
			})
		})
	})
	Describe("nccl_2.8.4test", func() {
		Context("when 1虚2", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.8.4-cuda11", vgpu1Map2, "2")
			})
		})
		Context("when 1虚2_1", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.8.4-cuda11", vgpu1Map21, "2")
			})
		})
		Context("when 1虚2_2", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.8.4-cuda11", vgpu1Map22, "2")
			})
		})
		Context("when 1虚3", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.8.4-cuda11", vgpu1Map3, "3")
			})
		})
		Context("when 1虚4", func() {
			It("log should contain keyword", func() {
				ncclTest("m7-ieg-pico-test01:5000/nccl-test:2.8.4-cuda11", vgpu1Map4, "4")
			})
		})
	})
})
