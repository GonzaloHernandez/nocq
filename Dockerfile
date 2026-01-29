FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    bison \
    flex \
    zlib1g-dev \
    libgecode-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && \
    cmake -DENABLE_GECODE=ON .. && \
    make -j$(nproc)

ENTRYPOINT ["./build/nocq"]
