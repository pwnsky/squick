FROM ubuntu:20.04
# Set env
ARG DEBIAN_FRONTEND=noninteractive
# The build enviroment for Squick 
RUN apt update && apt-get install -y git cmake unzip automake make g++ libtool libreadline6-dev libncurses5-dev pkg-config libssl-dev

# The build enviroment for Squick www server
RUN apt-get install -y nodejs npm libjsoncpp-dev uuid-dev zlib1g-dev

# Build Squick c++ source code
RUN cd ~ && git clone https://github.com/i0gan/Squick.git
RUN cd ~/Squick && chmod +x build.sh && chmod +x third_party/build.sh 
RUN cd ~/Squick/third_party && bash ./build.sh
RUN cd ~/Squick/ && bash ./build.sh

# Build Squick www source code
RUN cd ~/Squick/third_party/build/drogon && make install
RUN cd ~/Squick/www/admin && git config --global url."https://".insteadOf git:// && npm install
RUN cd ~/Squick/www && bash ./build.sh
