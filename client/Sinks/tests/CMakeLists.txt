add_executable(
  xdbc-client-tests
  test_sink.cpp
)

target_link_libraries(
  xdbc-client-tests
  xdbc-client-lib
  GTest::gtest_main
  pthread
)

include(GoogleTest)
gtest_discover_tests(xdbc-client-tests)
