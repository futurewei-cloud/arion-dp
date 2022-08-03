# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2022 The Authors.
# Authors:     Bin Liang     <@liangbin>
# Modified by: Wei Yue       <@w-yue>
#
# Summary: Customize arion_dev container for local build environments

#FROM fwnetworking/arion_dev:latest
FROM wyue/arion_dev:latest

# Set container user to match host $USER to avoid messing up shared folders
ARG USER_NAME
ARG USER_ID
ARG GROUP_ID

RUN if [ $USER_ID -ne 0 ]; then (getent group $GROUP_ID || addgroup --gid $GROUP_ID $USER_NAME) && \
    adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID $USER_NAME; fi

USER $USER_NAME
