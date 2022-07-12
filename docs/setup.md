## SetUp

### 0 Local
<<<<<<< HEAD

On your local pc, please set up the signaling server first.

=======
On your local pc, please set up the signaling server first.
>>>>>>> ef9eac5304347582e1e6d912677dd640c16b1444
```shell
git submodule update --init --recursive
cd server
python3 signaling-server.py
python3 -m http.server --bind 127.0.0.1 8080
```
<<<<<<< HEAD

And in your browser,type `127.0.0.1:8080`. After you have successfully run the program, press start button.

### 1 Ubuntu

Requires g++>=9.0.0 (any version which supports c++17 will be okay),and cmake>=3.15

=======
And in your browser,type `127.0.0.1:8080`. After you have successfully run the program, press start button.

### 1 Ubuntu
Requires g++>=9.0.0 (any version which supports c++17 will be okay),and cmake>=3.15
>>>>>>> ef9eac5304347582e1e6d912677dd640c16b1444
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
<<<<<<< HEAD

=======
>>>>>>> ef9eac5304347582e1e6d912677dd640c16b1444
```shell
brew install ffmpeg
cmake -B build
make -j2
./ors
```

### 3 Docker
Switch to docker branch first. The image maybe large(up to 2GB) because of ffmpeg and some other libraries. Please build in the root directory of this project.

<<<<<<< HEAD
Switch to docker branch first. The image maybe large(up to 2GB) because of ffmpeg and some other libraries. Please build in the root directory of this project.

=======
>>>>>>> ef9eac5304347582e1e6d912677dd640c16b1444
```shell
docker build -t ors-image
docker run --rm --name my-ors -p 8080:8080 -p 8000:8000/udp ors-image
```
<<<<<<< HEAD

Then in docker termianl, run

=======
Then in docker termianl, run
>>>>>>> ef9eac5304347582e1e6d912677dd640c16b1444
```shell
xvfb-run ./ors
```

### 4 Windows
<<<<<<< HEAD

=======
>>>>>>> ef9eac5304347582e1e6d912677dd640c16b1444
Windows currently has not been supported yet.
