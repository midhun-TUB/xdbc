# XDBC Monorepo :  `git filter-repo`

This monorepo merges `xdbc-client` and `xdbc-server` (branch: `test/reproduce`) using **`git filter-repo --to-subdirectory-filter`**.

## Structure

```
XDBC-filter/
  client/   ← xdbc-client source
  server/   ← xdbc-server source
```
To access the history of client (or server) use git log as given below
```bash
git log -- client/xdbc/xclient.cpp
```

## Running the Project

To build the combined image and spin up both the **client** and **server** containers :

```bash
# 1. Build the unified image
make

# 2. Start the infrastructure
docker compose up -d
```

This will create two containers (`xdbcserver` and `xdbcclient`) using the same `xdbc-unified:latest` image, mapping their shared `/dev/shm` volumes correctly.

Before running, download and extract the required dataset to `/dev/shm`:

```bash
# Download the ss13husallm dataset (~250 MB compressed, ~1.2 GB extracted)
wget -O ss13husallm.csv.tar.gz "https://tubcloud.tu-berlin.de/s/M3aeptL8R5ekWSD/download?path=%2F&files=ss13husallm.csv.tar.gz"

# Extract to /dev/shm (shared memory, accessible inside containers)
tar --overwrite -xzf ss13husallm.csv.tar.gz -C /dev/shm
```

You can then run commands inside them:

```bash
# Start the server
docker exec -it xdbcserver bash -c "./xdbc-server/build/xdbc-server"

# Run a client command
docker exec -it xdbcclient bash -c "/xdbc-client/Sinks/build/xdbcsinks --server-host=xdbcserver --table ss13husallm -f1 -b 1024 -p 32000 -n1 -w1 -d1 -s1 --skip-serializer=0 --target=csv"
```
