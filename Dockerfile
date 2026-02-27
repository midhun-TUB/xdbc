# postgres server 14 on ubuntu 22.04 image
FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get upgrade -qy

#-------------------------------------------- Install XDBC and prerequisites -------------------------------------------
# install arrow/parquet dependencies

RUN apt install -qy ca-certificates lsb-release wget pip

RUN wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb

RUN apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb

RUN apt update && apt install -qy cmake git gdb nlohmann-json3-dev clang libboost-all-dev build-essential libspdlog-dev iproute2 netcat libarrow-dev=18.1.0-1 libparquet-dev=18.1.0-1 libthrift-dev pkg-config

# install compression libs

RUN apt install -qy libzstd-dev liblzo2-dev liblz4-dev libsnappy-dev libbrotli-dev

RUN git clone https://github.com/lemire/FastPFor.git && cd FastPFor && git checkout 039134b && \
    mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build . --config Release && \
    make install

RUN ln -s /FastPFor /fastpfor

RUN git clone https://github.com/LLNL/fpzip.git && cd fpzip && \
    mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build . --config Release && \
    make install


# install postgres dependencies
RUN apt install -qy libpq-dev libpqxx-dev

# install clickhouse depencencies
RUN apt-get install -y libabsl-dev
RUN git clone https://github.com/google/cityhash.git
RUN cd /cityhash && ./configure && make all check CXXFLAGS="-g -O3" && make install

# install clickhouse-lib
RUN git clone https://github.com/ClickHouse/clickhouse-cpp.git
RUN cd /clickhouse-cpp && rm -rf build && mkdir build && cd build && cmake .. -DWITH_SYSTEM_ABSEIL=ON && make -j8 && make install

# install webserver for http experiments
RUN pip install rangehttpserver

#------------------------------------------------------------------------

# Copy the entire project context
RUN mkdir /xdbc
COPY . /xdbc/

# build xdbc
RUN mkdir /xdbc/build && cd /xdbc/build && cmake .. -D CMAKE_BUILD_TYPE=Release && make -j8 && make install

RUN mkdir /xdbc/build/client/Sinks/build && cd /xdbc/build/client/Sinks/build && cmake /xdbc/client/Sinks -D CMAKE_BUILD_TYPE=Release && make -j8

RUN ldconfig

ENTRYPOINT ["tail", "-f", "/dev/null"]
