# XDBC

- [XDBC](https://dl.acm.org/doi/10.1145/3725294) is a holistic, high-performance framework for fast and scalable data transfers across heterogeneous data systems (e.g. DBMS to dataframes) aiming to combine the generality of generic solutions with performance of specialized connectors
- It decomposes data transfer into a configurable pipeline (read -> deserialize -> compress -> send/receive -> decompress -> serialize -> write) with pipeline-parallel execution and ring-buffer memory manager for low resource overhead.
- The core of the framework (xdbc-client and xdbc-server) are written in C++ with bindings available for Python and Spark. It includes built-in adapters to connect to PostgreSQL, CSV, Parquet and Pandas.
- The project includes a lightweight heuristic optimizer implemented in Python that automatically tunes the parallelism, buffer sizes, intermediate formats and compression algorithms to the current environment.


## Project Structure

XDBC consists of multiple repositories covering the cross-system functionality. For the reproducibility experiments the following repositories will be cloned and used :

- [`xdbc-client`](https://github.com/polydbms/xdbc-client) Client-side module, for loading data into the target system.
- [`xdbc-server`](https://github.com/polydbms/xdbc-server) Server-side module, for extracting the data from the source system.
- [`xdbc-python`](https://github.com/polydbms/xdbc-python) Python bindings for loading data into Pandas (through pybind).
- [`xdbc-spark`](https://github.com/polydbms/xdbc-spark) Spark bindings, for loading data into a Spark RDD (through a custom DataSource with JNI).
- [`pg_xdbc_fdw`](https://github.com/polydbms/pg_xdbc_fdw) PostgreSQL Foreign Data Wrapper, for loading data into a table.
