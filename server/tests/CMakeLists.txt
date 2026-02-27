## Queue tests (no external deps beyond gtest)
add_executable(
  xdbc-server-tests
  test_queue.cpp
)

target_link_libraries(
  xdbc-server-tests
  GTest::gtest_main
  pthread
)

include(GoogleTest)
gtest_discover_tests(xdbc-server-tests)

## Compressor tests (need compression libraries)
add_executable(
  xdbc-compressor-tests
  test_compressor.cpp
)

target_link_libraries(
  xdbc-compressor-tests
  GTest::gtest_main
  pthread
  ${ZSTD_LIBRARY_PATH}
  ${SNAPPY_LIBRARY_PATH}
  lzo2
  lz4
  ZLIB::ZLIB
  spdlog::spdlog_header_only
)

gtest_discover_tests(xdbc-compressor-tests)

## Deserializer tests (header-only, needs fast_float)
add_executable(
  xdbc-deserializer-tests
  test_deserializers.cpp
)

target_include_directories(xdbc-deserializer-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(
  xdbc-deserializer-tests
  GTest::gtest_main
  pthread
)

gtest_discover_tests(xdbc-deserializer-tests)

## DataSource utility tests (need nlohmann/json, spdlog, DataSource.cpp)
add_executable(
  xdbc-datasource-tests
  test_datasource.cpp
  ../DataSources/DataSource.cpp
)

target_include_directories(xdbc-datasource-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(
  xdbc-datasource-tests
  GTest::gtest_main
  pthread
  spdlog::spdlog
)

gtest_discover_tests(xdbc-datasource-tests)
