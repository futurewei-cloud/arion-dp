#!/usr/bin/env bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2022 The Authors.
# Authors:     Bin Liang    <@liangbin>
# Modified by: Wei Yue      <@w-yue>
#
# Summary: script to setup k8s cluster and deploy Arion services
#

#set -x

# Change Ansible debug level: [-v|-vv|-vvv|-vvvv]
ANSIBLE_VERBOSE=""
playbook=""

# Save current location
myloc=$(pwd)
# Get full path of this script no matter where it's placed and invoked
MY_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# source deploy_common.sh
. $MY_PATH/deploy_common.sh

if [[ "$stage" == "development" || "$stage" == "production" ]]; then
  playbook="build_site create_k8s deploy_dependency publish_arion deploy_arion"
elif [[ "$stage" == "user" ]]; then
  playbook="build_site create_k8s deploy_dependency deploy_arion"
else
  playbook="build_site"
fi

echo "==== Full deployment request accepted: stage is $stage, site is $site, registry is $reg ====" >&2

cd $MY_PATH/playbooks
# Iterate the string variable using for loop
for play in $playbook; do
    echo $PLAY_CMD play_$play.yml | /bin/bash
done

rm -f $MY_PATH/.env_changed

# Go back to invoking folder
cd $myloc
