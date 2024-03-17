# Squick

A highly scalable, high-performance, high-availability, cross-platform (Windows, Linux, MacOS) server-side framework. This framework is co-written using C++23 and Lua languages, surport c++ & lua coroutine feature, and the framework is split into the kernel layer, node layer and business layer to improve the scalability and maintainability of the program. Split business functions to run on different nodes to achieve decoupling of different functions and efficient utilization of resources. Using dynamic load balancing algorithms, tasks can be effectively distributed among different nodes to ensure system stability and performance.

**Unity SDK**:  [squick-unity-sdk](https://github.com/pwnsky/squick-unity-sdk)

**Unreal SDK**: [squick-unreal-sdk](https://github.com/pwnsky/squick-unreal-sdk)

**Web Backstage**:  [squick-mid](https://github.com/pwnsky/squick-mid)

**QQ Group：739065686**

**Version:** 1.0.4

---

![GitHub release](https://img.shields.io/github/release/pwnsky/Squick.svg?style=flat-square)
![GitHub pre-release](https://img.shields.io/github/release-pre/pwnsky/Squick.svg?label=pre-release&style=flat-square)
[![Platform](https://img.shields.io/badge/Platform-Linux-green.svg?style=flat-square)](https://github.com/pwnsky/Squick)
[![C++23](https://img.shields.io/badge/C++-23-4c7e9f.svg?style=flat-square)](https://github.com/pwnsky/Squick)
[![License](https://img.shields.io/github/license/pwnsky/Squick.svg?colorB=f48041&style=flat-square)](https://opensource.org/licenses/Apache-2.0)
![GitHub stars](https://img.shields.io/github/stars/pwnsky/Squick.svg?style=flat-square&label=Stars&style=flat-square)
![GitHub issues](https://img.shields.io/github/issues-raw/pwnsky/Squick.svg?style=flat-square)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/pwnsky/Squick.svg?style=flat-square)


## Projects

[BiochemicalSecrets](https://github.com/pwnsky/BiochemicalSecrets)


## Feature

1. [10%] Distributed services, each service communicates through the network, Surport K8s cluster to manage node, and can reduce server pressure through distributed + clustering methods
2. [100%] Use Lua to develop IO-intensive business code, and C++ to develop CPU-intensive services, which can be used to develop combat suits.
3. [80%] The workload status of all nodes in the same region is self-synchronized.
4. [10%] Provide stable services and ensure high availability of services.
5. [20%] Optimize ultra-low CPU, memory, and network usage.
6. [10%] It has standardized tests that allow developers to self-test various performance indicators (CPU, memory, network).
7. [100%] Plug-in module combination turns development functions into plug-in modules for development and expansion.
8. [60%] Support non-stop hot updates of business(c++/lua) code.
9. [100%] When the program crashes in the Linux environment, the Dump Stack call chain is automatically written to the crash file.
10. [50%] 
11. [0%] Network library adds C++ coroutines
12. Supports cross-platform, can be compiled and developed on Windows and Linux and MacOS, and can support VS for project development on Windows【100%】
13. [100%] Supports changing 1 to N processes or N to 1 process. A single process on a physical machine can start all servers to facilitate development and debugging. Start single or multiple servers on different physical machines 
14. [10%] It has relatively complete automation tools, such as automated code generation, compilation, packaging, deployment, cleaning, etc. 【80%】
15. [60%] Supports mainstream protocols, such as HTTP/HTTPS, Websocket, TCP, UDP, KCP, etc.【60%】
16. [60%] In order to ensure the correct processing and non-loss of packets, a custom two-way RPC protocol is built on top of TCP.【100%】
18. [00%] Remote debugging console
20. [80%] Has client sdk for Unity and UnrealEngine
21. [80%] High availability - database proxy server, supports mainstream MongoDb, Mysql, Redis, Clickhouse databases etc.


## Server-side architecture

Connections

![img](./docs/images/object_connections.png)

RPC communication method between servers

<img src="./docs/images/server_to_server_rpc.png" alt="img" style="zoom: 33%;" />

# Squick Quick start

## Windows


## Linux


## MacOS


## Docker




## Friend link

[NoahGameFrame](https://github.com/ketoo/NoahGameFrame)

[Moon](https://github.com/sniper00/moon)

