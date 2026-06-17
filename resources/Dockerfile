FROM ubuntu:24.04

# Install the standard Ubuntu boost library
RUN apt-get update && apt-get install -y \
    zlib1g \
    libstdc++6 \
    libboost-iostreams-dev \
    bash \
    && rm -rf /var/lib/apt/lists/*

# Fix the Boost version discrepancy by creating a symbolic link
RUN ln -s /usr/lib/x86_64-linux-gnu/libboost_iostreams.so /usr/lib/x86_64-linux-gnu/libboost_iostreams.so.1.89.0

WORKDIR /usr/local/bin

# This copies files from your ~/Cloud/engines/ build context
COPY cadical .
COPY kissat .
COPY minisat .
COPY oink .
COPY nocq .

# Grant executable permissions to all binaries, including nocq
RUN chmod +x cadical kissat minisat oink nocq

CMD ["/bin/bash"]

# docker build -f Dockerfile -t solver ~/Cloud/engines/