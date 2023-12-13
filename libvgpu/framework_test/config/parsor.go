package config

import (
	"fmt"
	"log"
	"strconv"
	"strings"

	"github.com/BurntSushi/toml"
)

type Config struct {
	Params params
}

type params struct {
	DockerRegistry string
	ExamplesPath   string
	VgpuPath       string
	ShmSize        string
}

func GetConfig() *Config {
	var conf *Config
	path := "config.toml"
	if _, err := toml.DecodeFile(path, &conf); err != nil {
		log.Fatal(err)
	}
	return conf
}

func GetDockerRegistry() string {
	return GetConfig().Params.DockerRegistry
}

func GetExamplesPath() string {
	return GetConfig().Params.ExamplesPath
}

func GetVpuPath() string {
	return GetConfig().Params.VgpuPath
}

func GetShmSize() int64 {
	shmSize, err := strconv.ParseInt(GetConfig().Params.ShmSize, 10, 64)
	if err != nil {
		panic(err)
	}
	return shmSize
}

func GetNvidiaEnv(nvmap string) string {
	resenv := "NVIDIA_VISIBLE_DEVICES="
	for _, val := range strings.Split(nvmap, " ") {
		tmp := strings.Split(val, ":")[1]
		tmp = strings.Trim(tmp, " ")
		resenv = resenv + tmp + ","
	}
	resenv = strings.Trim(resenv, ",")
	fmt.Println("resenv=", resenv)
	return resenv
}
