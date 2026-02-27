# XDBC Development Guide: Testing Framework

This document outlines the testing strategies used in the XDBC monorepo. It explains the differences between our end-to-end (E2E) Docker-based testing and our component-level Unit Testing framework, and how to use both effectively during development.

---

### Framework Used
We use [Google Test (gtest)](https://google.github.io/googletest/). This is automatically downloaded and configured when you run CMake on the host machine. You do not need to install anything manually aside from a C++ compiler and CMake.

### Test Executables

#### Server Tests (`server/tests/`)

| Executable | Source File | What It Tests |
|---|---|---|
| `xdbc-server-tests` | `test_queue.cpp` | customQueue: FIFO ordering, capacity blocking, concurrency (multi-producer/multi-consumer), sentinel termination pattern |
| `xdbc-compressor-tests` | `test_compressor.cpp` | All 5 compression algorithms (zstd, snappy, lzo, lz4, zlib): compress, round-trip integrity, edge cases, integer column data |
| `xdbc-deserializer-tests` | `test_deserializers.cpp` | CSV deserializer templates: int, double, char, fixed-size string parsing, full CSV row deserialization |
| `xdbc-datasource-tests` | `test_datasource.cpp` | DataSource utilities: JSON schema parsing (`createSchemaFromJsonString`), `getSchemaSize`, `createSchemaAttribute` |

#### Client Tests (`client/Sinks/tests/`)

| Executable | Source File | What It Tests |
|---|---|---|
| `xdbc-client-tests` | `test_sink.cpp` | CSVSink `SerializeAttribute` templates (int, double, char, fixed-string), full tuple serialization, `RuntimeEnv.calculateTupleSize`, `RuntimeEnv.toString` |
| `xdbc-decompressor-tests` | `test_decompressor.cpp` | All 5 decompression algorithms, dispatch method routing, server-to-client round-trip integrity for text/integer/double data |
| `xdbc-pqsink-tests` | `test_pqsink.cpp` | `CreateParquetSchema` for all column types (INT, DOUBLE, STRING, CHAR), mixed schemas, error handling for unsupported types |
| `xdbc-utils-tests` | `test_utils.cpp` | `Utils::compute_checksum`, `boolVectorToString`, `boolVecAsStr`, `slStr` |

### How to Run Unit Tests Locally
on your host machine during active development.

```bash
# 1. Create a build directory
mkdir build && cd build

# 2. Run CMake (this downloads googletest automatically)
cmake ..

# 3. Build all test binaries
make -j$(nproc)

# 4. Run the full test suite via CTest
ctest --output-on-failure
```

To build and run a specific test target:

```bash
# Build only the compressor tests
make xdbc-compressor-tests

# Run it directly
./server/tests/xdbc-compressor-tests

# Or run via ctest with a filter
ctest -R Compressor --output-on-failure
```

### How to Write a New Unit Test

1. Create a `.cpp` file in the appropriate `tests/` directory and use the `TEST()` macro:

```cpp
#include <gtest/gtest.h>
#include "../your_component_header.h"

TEST(YourComponentTest, ExpectedBehaviorName) {
    int result = MyComponent::Add(2, 2);
    EXPECT_EQ(result, 4); // Assertion
}
```

2. Add your new file to the `CMakeLists.txt` inside that `tests/` directory. Either add it to an existing `add_executable` or create a new test target:

```cmake
add_executable(
  your-new-tests
  test_your_component.cpp
)

target_link_libraries(
  your-new-tests
  GTest::gtest_main
  pthread
  # ... any additional libraries your component needs
)

gtest_discover_tests(your-new-tests)
```

### Test Design Notes

- **Compression/Decompression tests** avoid pulling in the full `xdbcserver.h` dependency chain by calling the compression libraries directly with the same logic as `Compressor.cpp`. This keeps tests fast and dependency-free.
- **Round-trip tests** simulate the server-compress then client-decompress pipeline to verify data integrity across the network boundary.
- **Concurrency tests** in `customQueue` use multiple threads and verify blocking behavior, FIFO ordering, and the sentinel-based termination pattern used throughout the XDBC pipeline.
- **Serializer tests** validate that the templated `SerializeAttribute` functions produce correct CSV output for each data type.

---

## 2. End-to-End Testing (Docker)

**What is it?**
E2E testing spins up the entire XDBC Server, Postgres DB, and XDBC Client simultaneously to perform massive, real-world data transfers (like transferring the `ss13husallm` dataset).

**Why use it?**
- **Accuracy:** Guarantees that all micro-components connect and work reliably under realistic network and database loads.

### How to Run E2E Tests via Docker

Instead of testing isolation, you test the full pipeline.

```bash
# 1. Build the unified image
make

# 2. Start the infrastructure
docker compose up -d

# 3. Start the server daemon inside its container
docker exec -it xdbcserver bash -c "./xdbc-server/build/xdbc-server"

# 4. Run a transfer from the client container
docker exec -it xdbcclient bash -c "/xdbc-client/Sinks/build/xdbcsinks --server-host=xdbcserver --table ss13husallm -f1 -b 1024 -p 32000 -n1 -w1 -d1 -s1 --skip-serializer=0 --target=csv"
```
