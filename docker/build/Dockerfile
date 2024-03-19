FROM ubuntu:22.04
# Set env
ARG DEBIAN_FRONTEND=noninteractive
# The build enviroment for Squick 
RUN apt update
RUN apt-get install -y git cmake make g++
RUN apt-get install -y libtool libreadline-dev libncurses-dev pkg-config libssl-dev
RUN apt-get install -y python3
# Build Squick c++ source code
RUN cd ~ && git clone https://github.com/pwnsky/squick.git

WORKDIR ~/squick
RUN cd ~/squick/tools && chmod +x *.sh
RUN cd ~/squick/tools && ./build_third_party.sh
RUN cd ~/squick/tools && ./build_sqkctl.sh
RUN cd ~/squick/tools && ./build_squick.sh
RUN cd ~/squick/tools && ./generate_deploy.sh