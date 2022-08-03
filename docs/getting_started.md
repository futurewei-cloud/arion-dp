# Basic Usage

Arion DP framework includes two self-contained clusters:
- One k8s micro-service cluster hosting Arion DP control related services including
  - Arion DP control service such as arion-operator and arion-manager
  - Arion DP dependency services such as ingress, postgres
  - responsible for providing Arion DP NB API and managing Arion Data Plane
- One Arion DP Gateway Cluster for Arion Data plane
  - Made of bare-metal servers or VMs and tested on bare-metal servers
  - Arion Data Plane functionalities are mainly controlled by Arion DP control services via its NB RPC interface.

To start developing/deploying Arion DP system, you will need:
- a host as **deployer**:
  - to build and package Arion DP system
  - to deploy Arion DP to remote site
  - to manage or interact with deployed Arion DP system

- a target site to deploy Arion DP into as **site**:
  - The site has dedicated bare-metal servers/VMs for Arion DP control plane service and Arion data plane cluster
  - Arion **deployer** script will deploy and provision the two Arion clusters on the target Arion servers/VMs

## Prerequisites
**Deployer** host Linux environment:
- Ubuntu: tested with 22.04 LTS
- Python3 & Pip3
- [Docker](https://www.docker.com): tested with version 20.10.12. Make sure your login user ID is added in docker group so you can run docker without sudo:
  ```
  sudo groupadd docker
  sudo usermod -aG docker $USER
  newgrp docker
  ```
- [Kubectl](https://kubectl.docs.kubernetes.io): tested with version 1.22
- Python modules for remote deployment:
  ```
  pip3 install boto boto3 botocore ansible
  ```

Target **site** host Linux environment:
- Ubuntu server: tested with 22.04 LTS
- Currently, the dedicated Arion DP servers/VMs are configured to be within the same network as Compute Nodes.
- Default Kernel version of Ubuntu 22.04 LTS is 5.15.0.

## Build and Unit Testing
1. Clone the Arion DP repository:
    ```
    git clone --recurse-submodules git@github.com:futurewei-cloud/arion-dp.git
    cd arion-dp
    ```
2. Build and Run built-in Unit tests, all test cases should pass
    ```
    ./build.sh debug
    ```

## Deploy and Testing


#### Testing with Alcor-Control-Agent
More information about the [Alcor-Control-Agent](https://github.com/futurewei-cloud/alcor-control-agent).
Once Arion DP has been fully deployed and ready, we can run a ping test with the help of the ACA (Alcor-Control-Agent).
Before proceeding with the ACA test, first we will need to build its binaries.
From the Arion DP directory run the following
```
$ ./src/extern/alcor-control-agent/build/aca-machine-init.sh
```
Once ACA is done building successfully, the docker image aca_build0 is created. From the arion-dp directory, you can now run a simple ping test with the command below.
```
python3 -W ignore -m unittest  src/mgmt/tests/test_zeta_aca_ping.py
```
Above test creates two containers (aca_node_1 & aca_node_2) from the aca_build0 image. These two container nodes will ping each other handful of times. A successful run of the test shows the time in seconds it took to complete the test.

### Remote Bare-metal deployment

#### Deployment to Lab server pool as example

**Note**: Before proceeding with this scenario, please review and change site inventory templates according to your setup in follow files:
- deploy/playbooks/inventories/vars/site_lab.yml
- deploy/playbooks/inventories/hosts_lab

Simply run the script below in the Arion-dp directory to deploy Arion DP to the remote bare-metal site.

1. Setup secret vault to keep LAB access credentials stored locally encrypted:

```
$ ./deploy/env_setup.sh
```

2. Make necessary changes for the LAB site to be created for you in deploy/playbooks/inventories/vars/site_lab.yml and deploy/playbooks/inventories/hosts_lab
3. Setup LAB site clusters and deploy Zeta

```
$ ./deploy/full_deploy.sh -d lab
```
4. Once deployed, local kubeconfig is automatically updated for the new cluster, so you can use kubectl to manage the remote Arion DP control services
5. To remove Arion DP deployment:
```
$ ./deploy/full_deploy.sh -r lab
```
6. If you made some changes to Arion DP, you can re-deploy only the arion dp services to the target site created in step 3, this will be faster
```
$ ./deploy/arion_deploy.sh -d lab
```
7. Once running arion_deploy.sh is successful, you can run following commands for testing.
```
$ ./test/run.py
```
In run.py, it sets up compute nodes(which should have ACA set up in previous step) by creating docker containers in the CNs and does initial ping tests between docker containers from different compute nodes.
