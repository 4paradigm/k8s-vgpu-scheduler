package vgpu_test

import (
	"fmt"
	"testing"

	"github.com/onsi/ginkgo"
	"github.com/onsi/gomega"
	"gitlab.4pd.io/libvgpu/framework_test/clients"
)

var (
	UUIDs      []string
	vgpu1Map2  string
	vgpu1Map3  string
	vgpu1Map4  string
	vgpu1Map21 string
	vgpu1Map22 string
)

func TestE2E(t *testing.T) {
	RunE2ETest(t)
}

func RunE2ETest(t *testing.T) {
	gomega.RegisterFailHandler(ginkgo.Fail)
	ginkgo.RunSpecs(t, "E2E Test Suite")
}

var _ = ginkgo.BeforeSuite(func() {
	UUIDs = clients.GetUUIDs()
	fmt.Printf("UUIDs :%v\n", UUIDs)
	vgpu1Map2 = fmt.Sprintf("0:%s 1:%s", UUIDs[0], UUIDs[0])
	vgpu1Map21 = fmt.Sprintf("0:%s 1:%s", UUIDs[0], UUIDs[1])
	vgpu1Map22 = fmt.Sprintf("0:%s 1:%s", UUIDs[1], UUIDs[1])
	vgpu1Map3 = fmt.Sprintf("0:%s 1:%s 2:%s", UUIDs[0], UUIDs[0], UUIDs[0])
	vgpu1Map4 = fmt.Sprintf("0:%s 1:%s 2:%s 3:%s", UUIDs[0], UUIDs[0], UUIDs[0], UUIDs[0])
})
