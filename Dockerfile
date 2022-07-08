ARG JOBS=2

FROM dokken/ubuntu-20.04
RUN sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
RUN apt update && apt install -y gcc g++ make cmake pkg-config \
     libfreetype6-dev libavcodec-dev libavformat-dev libswscale-dev libavutil-dev libavdevice-dev \
     libxxf86vm-dev libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
     libssl-dev \
     xvfb

# Copy source code to docker /ors folder
COPY . /app
WORKDIR /app

# Build and make ors
RUN rm -r build && cmake clean . && cmake -B build && cd build && make -j${JOBS} 

# Update work dir
WORKDIR build

#ENTRYPOINT xvfb-run --server-args='-screen 0,1024*768*24'

EXPOSE 8080 8000/udp
