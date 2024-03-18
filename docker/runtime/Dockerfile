FROM ubuntu:22.04
# Set env
ARG DEBIAN_FRONTEND=noninteractive

RUN sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
RUN sed -i s@/security.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list

# The build enviroment for Squick 
#RUN apt update && apt-get install -y git cmake unzip automake make g++ libtool libreadline6-dev libncurses5-dev pkg-config libssl-dev

# The build enviroment for Squick www server
#RUN apt-get install -y nodejs npm libjsoncpp-dev uuid-dev zlib1g-dev

# The tools for development
#RUN apt-get install -y vim git

# Clean pakage
RUN apt clean
