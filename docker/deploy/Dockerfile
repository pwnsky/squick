FROM ubuntu:22.04
# Set env
ARG DEBIAN_FRONTEND=noninteractive
# The build enviroment for Squick 
RUN apt update && apt-get install -y git cmake unzip automake make g++ libtool libreadline-dev libncurses-dev pkg-config libssl-dev python3

# Build Squick c++ source code
RUN cd ~ && git clone https://github.com/i0gan/Squick.git
RUN cd ~/Squick && chmod +x build.sh && chmod +x third_party/build.sh 
RUN cd ~/Squick/third_party && bash ./build.sh
RUN cd ~/Squick/ && bash ./build.sh

# Build www c++ source code
