#! /bin/bash
curl https://raw.githubusercontent.com/helm/helm/master/scripts/get | bash

kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/v0.9.1/Documentation/kube-flannel.yml
kubectl apply -f /etc/kubeadm/device-plugin/nvidia-1.9.10.yml

kubectl taint nodes --all node-role.kubernetes.io/master-

docker info | grep nvidia && kubectl label nodes `kubectl get nodes -o=custom-columns=NAME:.metadata.name | sed -n '1!p'` hardware-type=NVIDIAGPU

for pod in `kubectl get pods -n kube-system -o=custom-columns=NAME:.metadata.name | grep nvidia-device-plugin-daemonset`; do
	gpu=$(kubectl exec -it -n kube-system $pod -- nvidia-smi -q | \
		grep 'Product Name' | head -n 1)

	label=$(echo $GPU | cut -d ':' -f 2 | xargs | tr '\r' '')
	label=$(echo ${label// /-})

	node=$(kubectl get pod -n kube-system $pod \
		-o=custom-columns=NODE:.spec.nodeName | tail -n 1)

	kubectl label nodes ${node} "nvidia.com/brand=${label}"
	kubectl label nodes ${node} hardware-type=NVIDIAGPU
done

red=$(tput setaf 1)
reset=$(tput sgr0)

echo "${red}Installing Helm, this will take a minute${reset}"
kubectl create serviceaccount tiller --namespace kube-system
kubectl create clusterrolebinding tiller-cluster-rule --clusterrole=cluster-admin --serviceaccount=kube-system:tiller
helm init --service-account tiller --wait

echo "${red}Installing the NVIDIA monitoring stack, this will take a minute${reset}"
helm repo add gpu-helm-charts https://nvidia.github.io/gpu-monitoring-tools/helm-charts
helm repo update
helm install gpu-helm-charts/prometheus-operator --name prometheus-operator --namespace monitoring
helm install gpu-helm-charts/kube-prometheus --name kube-prometheus --namespace monitoring
