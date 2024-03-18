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
RUN cd tools && chmod +x *.sh
RUN cd tools && ./build_third_party.sh && ./build_sqkctl.sh && ./build_squick.sh
RUN ./generate_deploy.sh