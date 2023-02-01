# Arion DP Benchmark in 100G environment
01/26/2023

v0.1
## Introduction

This is a benchmark report for Arion DP performance. We conducted the same the network throughput benchmark, latency benchmark and redis benchmark. The same as we did previously, but under 100G environment.

We used different tools to conduct various benchmark tests. The results shown below are collected with netperf tests. The throughput benchmark uses netperf's TCP_STREAM test. TCP_RR and TCP_CRR are used to observe network latency. We use redis's default benchmark suits for redis benchmark.

The main comparision in benchmarking is between direct communication and communication through Arion Cluster between compute nodes. 

## Test environment

The Ariond DP cluster is set up with: 
 - 2 bare-metal machines as Arion Wings;
 - 2 bare-matal machines as Compute Nodes.

Each Arion Wing in Arion cluster runs with Ubuntu 22.04. Compute Nodes run with Ubuntu 18.04.

  - CPU
    * Intel(R) Xeon(R) CPU E5-2660 v3 @ 2.60GHz 20 cores/40 threads(Wing)
    * Intel(R) Xeon(R) CPU E5-2620 v4 @ 2.10GHz 16 cores/32 threads(Compute Node)
  - Network Card: Mellanox ConnectX-6 100G DX
  - Kernel 
    * Linux Ubuntu 22.04 LTS (Wing)
    * Linux Ubuntu 18.04 LTS (Compute Node)
  - Tofino Switch
    * SMC Edgecore Wedge100BF-32X 32-Port 100G

Arion Wings and Compute Node are connected via 100G tofino switch. Each compute node launches docker containers and the communication is between containers in different compute nodes.

## Test setup

The tests shown here includes:
  - Direct(Node <-> Switch <-> Node): In this setup, no Arion Cluster is envolved. The benchmark is performed by directly running *netperf*  between docker containers in different bare metal machines with differnt MTUs:
    * MTU 1500
    * MTU 9000

  - Via Arion Cluster(Node <-> Switch <-> Wing <-> Switch <-> Node): In this setup, all traffic goes through a single Arion Wing, no direct path is allowed. Same *netperf* tests are conducted for following setup:
    * MTU 1500
    * MTU 9000

  - Redis benchmarking are conducted as following with default MTU 1500 and generic mode
    - Via Arion Cluster(Node <-> Switch <-> Wing <-> Switch <-> Node):
      - default redis-benchmarking
      - redis-benchmarking with larger packet size(1400 bytes)
    - Direct(Node <-> Switch <-> Node):
      - default redis-benchmarking
      - redis-benchmarking with larger packet size(1400 bytes)

## Summary of the Results

The tests are done with in house 100G network environment. Below are some observations about current Arion DP: 
 - Network delay to go through Arion Wing(CN-Arion-CN) in netperf TCP_RR test compared with direct CN-CN path can is about *15us*;
 - For single flow tcp_stream benchmark, traffic through Arion DP and direct reaches about the same throughput at about *13~14* Gbps with 9000 mtu;
 - For 32 flows tcp_stream benchmark, traffic through Arion DP reaches about *70.5 Gbps*, direct CN communication reaches about *76.6 Gbps*, which is about *8%* lower, we are looking into it for better understanding and potention improvement;
 - For default redis benchmark, through Arion DP reaches about *97%* of direct in terms of requests/s.


## The Throughput Benchmark 

Netperf TCP throughput metric measures the maximum data transfer rate between containers running on different nodes, which is the *netperf*'s default test. 

### single flow

| Config	  | MTU	   | Througphut(Mbps) | Difference(%) |  Retrans/s |
| :---      | :---:  | :---:            |         :---: |     ---:   |
| via arion |	1500   |	5674.53         | 	100.90%     |   148.08   |
| direct	  |	1500   |	5623.93 	      |	  100.00%     |     0.77   |
| via arion |	9000   |	13044.08 		    |    95.08%     |     10.96  |
| direct	  | 9000   |	13719.46 		    |    100.00%    |     3.17   |

<img src="https://user-images.githubusercontent.com/83482178/215832641-5e294200-2d0b-4270-8b1e-a011d60eda5c.png" width=75%>

The above graph shows the maximum throughput that can be achieved with a single TCP connection. With MTU set as 9000, both Node to Node and via Arion reaches over *13* Gbps for 100G Nic interface; With default MTU(1500), via arion and direct reaches about the same throughput of less than *6* Gbps.

### multiple flows

| config    | MTU  | 1 flow(Mbps)| 8 flows(Mbps)  | 16 flows(Mbps) |  32 flows(Mbps) |  64 flows(Mbps) |
| :---      | :---:| :---:     | :---:    | :---:    |     :---: |    ---:   |
| via arion |	1500 |	5603.42  | 32846.11 |	37712.42 |	48277.81 |	48858.52 |
| direct	  | 1500 |	5665.26	 | 36555.08 |	44325.32 |	49040.68 |	44639.58 |
| via arion |	9000 |	14105.79 | 67274.03 |	70467.09 |	70604.22 |	69111.35 |
| direct	  | 9000 |	14180.46 | 74302.11 |	77693.58 |	76589.89 |	72069.28 |

<img src="https://user-images.githubusercontent.com/83482178/215843305-b03689a2-fa90-47d9-96d1-4453e74b8e34.png" width=75%>

The above graph shows the maximum throughput that can be achieved with multiple TCP connections.

TCP throughput benchmark is extreamly useful for applications like:
 - AL/ML applications which requre access to large amount of data;
 - Media streaming services.

## Latency: Requests per Second
The request per second metric measures the rate of single byte round-trips that can be performed in sequence over a single persistent TCP connection. It can be thought of as a user-space to user-space ping with no think time - it is by default a synchronous, one transaction at a time, request/response test.
This benchmark highlights how effeciently a single network packet can be processed.


<img src="https://user-images.githubusercontent.com/83482178/215834203-769d3dab-b56a-4ef0-815a-f5aedb945f75.png" width=75%>

<img src="https://user-images.githubusercontent.com/83482178/215834313-0e31abcb-b240-4831-a924-6ac55c285bd0.png" width=75%>



## Latency: Rate of new Connections
This test measures the performance of establishing a connection, exchanging a single request/response transaction, and tearing-down that connection. This is very much like what happens in an HTTP 1.0 or HTTP 1.1 connection when HTTP Keepalives are not used.

<img src="https://user-images.githubusercontent.com/83482178/215834496-a81196be-edd4-4150-9b29-2668c9761be8.png" width=75%>

<img src="https://user-images.githubusercontent.com/83482178/215834595-1b120feb-3d0d-4115-bde9-4eafc9b68fe9.png" width=75%>

## Redis Benchmarking

### Throughput(Request/s)
To experiement how well application may run across Arion DP cluster, we run standard redis benchmarking tool on Compute Nodes and compare the performance between direct compute node to compute node and via Arion DP cluster. With redis server run on one of the containers in compute node, redis benchmarking application is launched from another container in different compute node with two sets of benchmarking tests. 
  - On one container, launch redis server with command:
    - *redis-server --protected-mode no*
  - On another container on different compute node, we launch redis benchmarking commands:
    - *redis-benchmark -h {server_ip} -n 1000000*;
      - default parameters:  
        - 1000000 requests
        - 50 parallel clients
        - 3 bytes payload
        - keep alive: 1
    - *redis-benchmark -h {server_ip} -d 1400 -P 50 -n 1000000*.  We use larger packet size and turn on pipeline for better throughput.
      - parameters:
        - 1000000 requests
        - 50 parallel clients
        - 50 pipeline requests
        - 1400 bytes in payload
        - keep alive: 1
    
<img src="https://user-images.githubusercontent.com/83482178/215834871-8fe20481-697f-4d28-9a7c-ac6ae1192a7f.png" width=80%>

<img src="https://user-images.githubusercontent.com/83482178/215834946-c0c427a5-82e2-4f55-88ca-5f2e20548932.png" width=80%>

| Test	     | default direct(req/s) |	default via arion(req/s)| difference(%)	| pipelined direct(req/s)	| pipelined via arion(%) | difference(%) |
| :---        | :---:     | :---:     |   :---: | :---:      |  :---:     | ---:    |
|PING_INLINE	| 51719.68	| 50253.78	| 97.17%	|	584941.5	 | 576541.81 	|	98.56%  |
|PING_BULK	  | 51942.65	| 50362.61	| 96.96%	| 1114827.12 | 1120156.75	| 100.48% |
|SET	        | 51570.31	| 50497.4	  | 97.92%	| 275031.34  | 282963.5	  | 102.88% |
|GET	        | 51875.29	| 50545.89	| 97.44%	|	378406.38	 | 423145.09	| 111.82% |
|INCR	        | 52010.2	  | 50720.23	| 97.52%	|	714285.75	 | 696242.12	|	97.47%  |
|LPUSH	      | 52031.84	| 50784.62	| 97.60%	|	188150.67  | 195854.98	| 104.09% |
|RPUSH	      | 52162.12	| 50666.26	| 97.13%	|	196891.72	 | 199302.64	| 101.22% |
|LPOP	        | 51937.26	| 50540.79	| 97.31%	|	262031.44	 | 240293.59	| 91.70%  |
|RPOP	        | 51829.58	| 50474.46	| 97.39%	|	261868.61	 | 250978.67	| 95.84%  |
|SADD	        | 52175.73	| 50787.2	  | 97.34%	|	658360.75	 | 649772.56	| 98.70%  |
|HSET	        | 52126.77	| 50697.08	| 97.26%	|	259844.64	 | 260693	    | 100.33% |
|SPOP	        | 52186.62	| 50704.8	  | 97.16%	|	807916	   | 789344.94	| 97.70%  |
|ZADD	        | 52339.58	| 50697.08	| 96.86%	|	474419.16	 | 480196.84	| 101.22% |
|ZPOPMIN	    | 52077.91	| 50800.1	  | 97.55%	|	801924.62	 | 842881.19	| 105.11% |
|LRANGE_100	  | 27410.03	| 26977.45	| 98.42%	|	3816.69	   | 3828.94	  | 100.32% |
|LRANGE_300	  | 12147.13	| 12055.16	| 99.24%	|	1194.6	   | 1201.73	  | 100.60% |
|LRANGE_500	  | 8411.28	  | 8377.03	  | 99.59%	|	755.39	   | 780.09	    | 103.27% |
|LRANGE_600	  | 6628.97	  | 6620.54	  | 99.87%	|	572.1	     | 575.44	    | 100.58% |
|MSET(10 key) |	51435.04	| 50764	    | 98.70%	|	32791.32	 | 34037.1	  | 103.80% |


Redis benchmarking results show that redis commands with default parameters through Arion DP cluster are at most about *3%* slower in terms of request/s compared to direct.

### Latency
The earlier version of redis-benchmark has latency and throughput in summary output but the later version only has requests/s output: requests/s and request time distribution. We also used "redis-cli --latency" to compare the average latency between direct and via arion. For redis application in our setup, via arion adds about *0.015 ms* latency in avarage in 100G environment.

For direct:

root@8f8ce443c29d:/# redis-cli --latency-history -h x.x.x.x

    min: 0, max: 1, avg: 0.07 (1480 samples) -- 15.00 seconds range

    min: 0, max: 1, avg: 0.07 (1479 samples) -- 15.00 seconds range

    min: 0, max: 1, avg: 0.07 (1478 samples) -- 15.00 seconds range

    min: 0, max: 1, avg: 0.08 (1477 samples) -- 15.01 seconds range
    
    min: 0, max: 1, avg: 0.09 (1477 samples) -- 15.00 seconds range
    
    min: 0, max: 1, avg: 0.08 (1477 samples) -- 15.00 seconds range
    
    min: 0, max: 1, avg: 0.09 (1478 samples) -- 15.01 seconds range
    
    min: 0, max: 1, avg: 0.08 (1477 samples) -- 15.00 seconds range

For via arion:

root@8f8ce443c29d:/# redis-cli --latency-history -h x.x.x.x

    min: 0, max: 1, avg: 0.10 (1478 samples) -- 15.01 seconds range

    min: 0, max: 1, avg: 0.10 (1476 samples) -- 15.01 seconds range

    min: 0, max: 1, avg: 0.10 (1476 samples) -- 15.01 seconds range

    min: 0, max: 1, avg: 0.10 (1476 samples) -- 15.01 seconds range
    
    min: 0, max: 1, avg: 0.10 (1475 samples) -- 15.01 seconds range
    
    min: 0, max: 1, avg: 0.10 (1476 samples) -- 15.01 seconds range
    
    min: 0, max: 1, avg: 0.10 (1476 samples) -- 15.00 seconds range
    
    min: 0, max: 1, avg: 0.10 (1476 samples) -- 15.01 seconds range

### Notes 

[Redis commands explanation](https://redis.io/commands/)
