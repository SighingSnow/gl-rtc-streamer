ARG JOBS = 2

FROM dokken/ubuntu-20.04
RUN apt install build-essentials gcc g++ make 

# Copy source code to docker
COPY . /ors
WORKDIR /ors/build

# Build and install ors
RUN cmake . && make -j${JOBS} && make install

EXPOSE 8080 8000/udp

# Default workdir and command
