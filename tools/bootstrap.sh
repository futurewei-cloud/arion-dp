#!/bin/bash

sudo apt-get update
sudo apt-get install -y \
    build-essential clang-7 llvm-7 \
    libelf-dev \
    python3 \
    python3-pip \
    libcmocka-dev \
    lcov

sudo apt install docker.io
sudo pip3 install netaddr docker scapy
sudo pip3 install grpcio-tools
sudo systemctl unmask docker.service
sudo systemctl unmask docker.socket
sudo systemctl start docker
sudo systemctl enable docker

ver=$(curl -s https://api.github.com/repos/kubernetes-sigs/kind/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
curl -Lo kind "https://github.com/kubernetes-sigs/kind/releases/download/$ver/kind-$(uname)-amd64"
chmod +x ./kind
sudo mv ./kind /usr/local/bin

curl -LO "https://storage.googleapis.com/kubernetes-release/release/$(curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt)/bin/linux/amd64/kubectl"
chmod +x ./kubectl
sudo mv ./kubectl /usr/local/bin/kubectl

git submodule update --init --recursive

kernel_ver=`uname -r`
echo "Running kernel version: $kernel_ver"

if [ "$kernel_ver" != "5.6.0-rc2" ]; then
	echo "Arion requires an updated kernel: linux-5.6-rc2 for TCP to function correctly. Current version is $kernel_ver"

	read -p "Execute kernel update script (y/n)?" choice
	case "$choice" in
	  y|Y ) sh ./tools/kernelupdate.sh;;
	  n|N ) echo "Please run ./tools/kernelupdate.sh to download and update the kernel!"; exit;;
	  * ) echo "Please run ./tools/kernelupdate.sh to download and update the kernel!"; exit;;
	esac
fi
ver=$(curl -s https://api.github.com/repos/kubernetes-sigs/kind/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
curl -Lo kind https://github.com/kubernetes-sigs/kind/releases/download/$ver/kind-$(uname)-amd64
chmod +x kind
mv kind /usr/local/bin
