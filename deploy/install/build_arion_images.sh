#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2022 The Authors.
# Authors:     Bin Liang    <@liangbin>
# Modified by: Wei Yue      <@w-yue>
#
# Summary: Rebuild and tag Arion service images for deployment
#
set -o errexit

# Bypass if it's "user" stage
if [[ ! -z "$STAGE" && "$STAGE" == "user" ]]; then
    exit 0
fi

# Get full path of current script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

echo "Rebuild arion-operator image..."
docker image build -t $REG/arion_opr:latest \
    -f $ROOT/deploy/etc/docker/operator.Dockerfile $ROOT >/dev/null

echo "Rebuild arion-manager image..."
docker image build -t $REG/arion_manager:latest \
    -f $ROOT/deploy/etc/docker/manager.Dockerfile $ROOT >/dev/null

if [[ "$REG" == "localhost:32000" ]]; then
    echo "Archiving Arion service images..."
    docker save $REG/arion_opr:latest $REG/arion_manager:latest > /tmp/arion_images.tar
fi
