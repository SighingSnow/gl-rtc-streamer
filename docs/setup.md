## SetUp

### 0 Local
On your local pc, please set up the signaling server first.
```shell
git submodule update --init --recursive
cd client
python3 signaling-server.py
python3 -m http.server --bind 127.0.0.1 8080
```
And in your browser,type `127.0.0.1:8080`. After you have successfully run the program, press start button.

### 1 Ubuntu
Requires g++>=9.0.0 (any version which supports c++17 will be okay),and cmake>=3.15
```shell
sudo apt install libfreetype6-dev libavcodec-dev libavformat-dev libswscale-dev libavutil-dev libavdevice-dev libxxf86vm-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libssl-dev
```

In the project root directory.

```
cmake -B build
cd build
make -j2
./ors
```

### 2 MacOS

Any issue is welcome(I cannot remember the details on macos).
```shell
brew install ffmpeg
cmake -B build
make -j2
./ors
```

### 3 Docker
The image maybe large(up to 2GB) because of ffmpeg and some other libraries. Please build in the root directory of this project.

```shell
docker build -t ors-image
docker run --rm --name my-ors -p 8080:8080 -p 8000:8000/udp ors-image
```
Then in docker termianl, run
```shell
xvfb-run ./ors -docker
```

### 4 Windows
Windows currently has not been supported yet.
