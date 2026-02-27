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
