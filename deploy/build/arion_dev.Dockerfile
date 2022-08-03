# SPDX-License-Identifier: MIT
# This file is originated from Zeta project
#
# Copyright (c) 2022 The Authors.
# Authors: Wei Yue           <@w-yue>
#
# Summary: arion_dev base container for development and dev test

FROM ubuntu:20.04

RUN apt-get update && \
    apt-get install -y apt-transport-https ca-certificates gnupg software-properties-common wget && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null && \
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

RUN apt-get update && \
    # Add ppa for gcc-9
    DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    sudo \
    build-essential \
    cmake \
    python3 \
    python3-pip \
    python3-setuptools \
    gdb \
    # Following are required for XDP development environment
    make \
    gcc-9 \
    g++-9 \
    gcc-9-multilib \
    g++-9-multilib \
    libssl-dev \
    libelf-dev \
    libcap-dev \
    clang-10 \
    llvm-10 \
    libncurses5-dev \
    git \
    pkg-config \
    libmnl-dev \
    bison \
    flex \
    graphviz \
    # End of XDP tools
    rpcbind \
    rsyslog \
    unzip \
    doxygen \
    zlib1g-dev \
    libboost-program-options-dev \
    libboost-all-dev \
    iproute2  \
    net-tools \
    iputils-ping \
    ethtool \
    curl \
    netcat \
    libcmocka-dev \
    lcov \
    autoconf \
    automake \
    dh-autoreconf \
    libtool && \
    apt-get clean

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 10 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 10

RUN pip3 install httpserver netaddr grpcio grpcio-tools flask

ENV PATH=$PATH:/usr/local/share/openvswitch/scripts \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
