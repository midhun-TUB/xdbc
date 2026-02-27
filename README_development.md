# XDBC Development Guide: Testing Framework

This document outlines the testing strategies used in the XDBC monorepo. It explains the differences between our end-to-end (E2E) Docker-based testing and our component-level Unit Testing framework, and how to use both effectively during development.

---

## 1. Unit Testing (Google Test)

**What is it?**
Unit tests verify that small, individual, isolated parts of the codebase (like a specific queue, or a single function reading a CSV) behave correctly independently of the rest of the system.

**Why use it?**
- **Speed:** Tests compile and execute locally in milliseconds. You don't wait for networks or databases to start.
- **Precision:** If a test fails, `Google Test` tells you exactly which line of assertions failed.
- **Simplicity:** You don't need to spin up the entire server and client containers just to test if a new class compiles and works.

### Framework Used
We use [Google Test (gtest)](https://google.github.io/googletest/). This is automatically downloaded and configured when you run CMake on the host machine. You do not need to install anything manually aside from a C++ compiler and CMake.

### Where are the tests located?
- **Server:** `server/tests/` (e.g., `test_queue.cpp`)
- **Client:** `client/Sinks/tests/` (e.g., `test_sink.cpp`)

### How to Run Unit Tests Locally

We recommend running unit tests directly on your host machine during active development.

```bash
# 1. Create a build directory
mkdir build && cd build

# 2. Run CMake (this downloads googletest automatically)
cmake ..

# 3. Build the test binaries (and the rest of the codebase)
make -j$(nproc)

# 4. Run the test suite via CTest
ctest -V
```

### How to Write a New Unit Test

Create a `.cpp` file in the appropriate `tests/` directory and use the `TEST()` macro.

```cpp
#include <gtest/gtest.h>
#include "../your_component_header.h"

TEST(YourComponentTest, ExpectedBehaviorName) {
    int result = MyComponent::Add(2, 2);
    EXPECT_EQ(result, 4); // Assertion
}
```

Then, add your new file to the `add_executable` list in the `CMakeLists.txt` inside that same `tests/` directory!

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
