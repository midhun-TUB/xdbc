include(GoogleTest)

find_package(Arrow REQUIRED)
find_library(PARQUET_SHARED_LIB parquet REQUIRED)

## CSVSink serializer + RuntimeEnv tests
add_executable(
  xdbc-client-tests
  test_sink.cpp
)

target_link_libraries(
  xdbc-client-tests
  xdbc-client-lib
  GTest::gtest_main
  pthread
  -larrow
  ${PARQUET_SHARED_LIB}
)

gtest_discover_tests(xdbc-client-tests)

## Decompressor + round-trip tests
add_executable(
  xdbc-decompressor-tests
  test_decompressor.cpp
)

target_link_libraries(
  xdbc-decompressor-tests
  xdbc-client-lib
  GTest::gtest_main
  pthread
  ${ZSTD_LIBRARY_PATH}
  ${SNAPPY_LIBRARY_PATH}
  lzo2
  lz4
  ZLIB::ZLIB
)

gtest_discover_tests(xdbc-decompressor-tests)

## PQSink schema creation tests
add_executable(
  xdbc-pqsink-tests
  test_pqsink.cpp
  ../PQSink/PQSink.cpp
)

target_link_libraries(
  xdbc-pqsink-tests
  xdbc-client-lib
  GTest::gtest_main
  pthread
  spdlog::spdlog
  -larrow
  ${PARQUET_SHARED_LIB}
)

gtest_discover_tests(xdbc-pqsink-tests)

## Utils tests (utils.h needs Boost for boost::asio and boost::crc)
find_package(Boost 1.40.0 REQUIRED system)

add_executable(
  xdbc-utils-tests
  test_utils.cpp
)

target_include_directories(xdbc-utils-tests PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(
  xdbc-utils-tests
  xdbc-client-lib
  GTest::gtest_main
  pthread
  ${Boost_SYSTEM_LIBRARY}
)

gtest_discover_tests(xdbc-utils-tests)
