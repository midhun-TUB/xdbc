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

To build the combined image and spin up both the **client** and **server** containers alongside a PostgreSQL instance:

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

## Filter Method merge details: git filter-repo

**Every historical commit is rewritten** so that all file paths are prefixed with `client/` or `server/`. The commits, authors, and dates stay the same — only the paths inside them change. The rewritten repos are then merged into this monorepo.

```bash
git filter-repo --to-subdirectory-filter client   # rewrites xdbc-client history
git filter-repo --to-subdirectory-filter server   # rewrites xdbc-server history
git merge client/test/reproduce --allow-unrelated-histories
git merge server/test/reproduce --allow-unrelated-histories
```

## Advantage: Full per-file history traceability

Because every old commit now uses the `client/` / `server/` prefix, `git log` and `git blame` work correctly across the entire history.


In contrast, `git subtree` would return only the single bulk merge commit for the same query.

## Trade-off: Upstream syncing is broken

If `xdbc-client` or `xdbc-server` gets new commits, you **cannot** pull them cleanly:

```bash
# This will conflict or fail — commit IDs no longer align
git merge client/test/reproduce --allow-unrelated-histories
```

