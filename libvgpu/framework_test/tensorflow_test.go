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

func transformerTest(NVIDIA_DEVICE_MAP string) {
	ctx := context.Background()
	img := fmt.Sprintf("%s/tensorflow-transformer:2.6.0rc0-gpu", GetDockerRegistry())
	cmd := []string{"sh", "-c", "python /examples/models/official/nlp/transformer/transformer_test.py 2>&1"}
	env := []string{
		GetNvidiaEnv(NVIDIA_DEVICE_MAP),
		fmt.Sprintf("NVIDIA_DEVICE_MAP=%s", NVIDIA_DEVICE_MAP),
		"LD_PRELOAD=/usr/local/vgpu/libvgpu_2.so",
		"CUDA_OVERSUBSCRIBE=true",
		"PARAM_SET=big",
		"DATA_DIR=$HOME/transformer/data",
		"MODEL_DIR=$HOME/transformer/model_$PARAM_SET",
		"VOCAB_FILE=$HOME/transformer/data/vocab.ende.32768",
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
	Expect(log).Should(ContainSubstring("[       OK ] TransformerV2Test.test_create_model_not_train"))
	Expect(log).Should(ContainSubstring("[       OK ] TransformerV2Test.test_create_model_train"))
	Expect(log).Should(ContainSubstring("[       OK ] TransformerV2Test.test_export"))
}

func distMirrStrategyTest(NVIDIA_DEVICE_MAP string) {
	ctx := context.Background()
	img := fmt.Sprintf("%s/tensorflow-transformer:2.6.0rc0-gpu", GetDockerRegistry())
	cmd := []string{"sh", "-c", "pip install matplotlib && python /examples/tf_distribute_mirroredStrategy.py 2>&1"}
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
	Expect(log).Should(ContainSubstring("Test accuracy:"))
}

func mnistMultiGPUTest(NVIDIA_DEVICE_MAP string) {
	ctx := context.Background()
	img := "tensorflow/tensorflow:1.15.5-gpu"
	cmd := []string{"sh", "-c", "python /examples/tf_mnist_multigpu.py 2>&1"}
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
	fmt.Printf("log: %s\n", log[len(log)-2000:])
	Expect(log[len(log)-2000:]).Should(ContainSubstring("Done"))
	Expect(log[len(log)-2000:]).Should(ContainSubstring("Testing Accuracy:"))
}

var _ = Describe("tensorflow", func() {
	Describe("Tensorflow 2.6 Mirrored Strategy+ Transformer", func() {
		Context("1虚2", func() {
			It("log should contain keyword", func() {
				transformerTest(vgpu1Map2)
			})
		})
		Context("1虚2_1", func() {
			It("log should contain keyword", func() {
				transformerTest(vgpu1Map21)
			})
		})
		Context("1虚2_2", func() {
			It("log should contain keyword", func() {
				transformerTest(vgpu1Map22)
			})
		})
		Context("1虚3", func() {
			It("log should contain keyword", func() {
				transformerTest(vgpu1Map3)
			})
		})
		Context("1虚4", func() {
			It("log should contain keyword", func() {
				transformerTest(vgpu1Map4)
			})
		})
	})
	Describe("Tensorflow2.6 Multi-GPU", func() {
		Context("1虚2", func() {
			It("log should contain keyword", func() {
				distMirrStrategyTest(vgpu1Map2)
			})
		})
		Context("1虚2_1", func() {
			It("log should contain keyword", func() {
				distMirrStrategyTest(vgpu1Map21)
			})
		})
		Context("1虚2_2", func() {
			It("log should contain keyword", func() {
				distMirrStrategyTest(vgpu1Map22)
			})
		})
		Context("1虚3", func() {
			It("log should contain keyword", func() {
				distMirrStrategyTest(vgpu1Map3)
			})
		})
		Context("1虚4", func() {
			It("log should contain keyword", func() {
				distMirrStrategyTest(vgpu1Map4)
			})
		})
	})
	Describe("Tensorflow1.15 Multi-GPU", func() {
		Context("1虚2", func() {
			It("log should contain keyword", func() {
				mnistMultiGPUTest(vgpu1Map2)
			})
		})
		Context("1虚2_1", func() {
			It("log should contain keyword", func() {
				mnistMultiGPUTest(vgpu1Map21)
			})
		})
		Context("1虚2_2", func() {
			It("log should contain keyword", func() {
				mnistMultiGPUTest(vgpu1Map22)
			})
		})
		Context("1虚3", func() {
			It("log should contain keyword", func() {
				mnistMultiGPUTest(vgpu1Map3)
			})
		})
		Context("1虚4", func() {
			It("log should contain keyword", func() {
				mnistMultiGPUTest(vgpu1Map4)
			})
		})
	})
})
