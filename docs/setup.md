## SetUp Instructions

### 1 Ubuntu
Requires g++>=9.0.0 (any version which supports c++17 will be okay)
```shell
sudo apt install libfreetype6-dev libavcodec-dev libavformat-dev libswscale-dev libavutil-dev libavdevice-dev libxxf86vm-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libssl-dev
```

### 2 MacOS

### 3 Docker
Switch to docker branch first.
```shell
docker build -t ors-image
docker run --rm -p 8080:8080 -p 8000:8000/udp ors-image ./build/ors
```

### 4 Windows
Windows currently has not been tested.
