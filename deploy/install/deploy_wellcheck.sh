#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2022 The Authors.
# Authors:        Bin Liang   <@liangbin>
# Modified by:    Wei Yue     <@w-yue>

# Summary: Script to check deployment of Arion services on target k8s cluster
#
set -o errexit

# Get full path of current script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

. $ROOT/deploy/install/common.sh

timeout=120
objects=("pods")
end=$((SECONDS + $timeout))
echo -n "Waiting for Arion control plane services to come up."
while [[ $SECONDS -lt $end ]]; do
    check_ready "${objects[@]}" "Running" || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Arion control plane services are ready!"
else
    echo "ERROR: Arion control plane services deployment timed out after $timeout seconds!"
    exit 1
fi

sleep 10s
