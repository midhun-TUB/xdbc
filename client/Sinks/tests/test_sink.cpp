#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>

#include "../CSVSink/CSVSink.h"
#include "../../xdbc/RuntimeEnv.h"

// =============================================
// Tests for SerializeAttribute<int>
// =============================================

TEST(SerializeAttributeTest, IntPositive) {
    int value = 12345;
    char buffer[64] = {};
    size_t written = SerializeAttribute<int>(&value, buffer, 0, ',');
    EXPECT_EQ(std::string(buffer, written), "12345,");
}

TEST(SerializeAttributeTest, IntNegative) {
    int value = -42;
    char buffer[64] = {};
    size_t written = SerializeAttribute<int>(&value, buffer, 0, ',');
    EXPECT_EQ(std::string(buffer, written), "-42,");
}

TEST(SerializeAttributeTest, IntZero) {
    int value = 0;
    char buffer[64] = {};
    size_t written = SerializeAttribute<int>(&value, buffer, 0, ',');
    EXPECT_EQ(std::string(buffer, written), "0,");
}

TEST(SerializeAttributeTest, IntNewlineDelimiter) {
    int value = 99;
    char buffer[64] = {};
    size_t written = SerializeAttribute<int>(&value, buffer, 0, '\n');
    EXPECT_EQ(std::string(buffer, written), "99\n");
}

// =============================================
// Tests for SerializeAttribute<double>
// =============================================

TEST(SerializeAttributeTest, DoublePositive) {
    double value = 123.45;
    char buffer[64] = {};
    size_t written = SerializeAttribute<double>(&value, buffer, 0, ',');
    EXPECT_EQ(std::string(buffer, written), "123.45,");
}

TEST(SerializeAttributeTest, DoubleNegative) {
    double value = -3.14;
    char buffer[64] = {};
    size_t written = SerializeAttribute<double>(&value, buffer, 0, ',');
    EXPECT_EQ(std::string(buffer, written), "-3.14,");
}

TEST(SerializeAttributeTest, DoubleZero) {
    double value = 0.0;
    char buffer[64] = {};
    size_t written = SerializeAttribute<double>(&value, buffer, 0, ',');
    EXPECT_EQ(std::string(buffer, written), "0.00,");
}

TEST(SerializeAttributeTest, DoubleWholeNumber) {
    double value = 100.0;
    char buffer[64] = {};
    size_t written = SerializeAttribute<double>(&value, buffer, 0, '\n');
    EXPECT_EQ(std::string(buffer, written), "100.00\n");
}

// =============================================
// Tests for SerializeAttribute<char>
// =============================================

TEST(SerializeAttributeTest, CharValue) {
    char value = 'A';
    char buffer[64] = {};
    size_t written = SerializeAttribute<char>(&value, buffer, 0, ',');
    EXPECT_EQ(written, 2);
    EXPECT_EQ(buffer[0], 'A');
    EXPECT_EQ(buffer[1], ',');
}

TEST(SerializeAttributeTest, CharNewline) {
    char value = 'Z';
    char buffer[64] = {};
    size_t written = SerializeAttribute<char>(&value, buffer, 0, '\n');
    EXPECT_EQ(written, 2);
    EXPECT_EQ(buffer[0], 'Z');
    EXPECT_EQ(buffer[1], '\n');
}

// =============================================
// Tests for SerializeAttribute<const char *> (fixed-size strings)
// =============================================

TEST(SerializeAttributeTest, FixedString) {
    const char str[10] = "hello";
    char buffer[64] = {};
    size_t written = SerializeAttribute<const char *>(str, buffer, 10, ',');
    // strnlen("hello", 10) = 5, so written = 5 + 1 = 6
    EXPECT_EQ(written, 6);
    EXPECT_EQ(std::string(buffer, 5), "hello");
    EXPECT_EQ(buffer[5], ',');
}

TEST(SerializeAttributeTest, FixedStringFullLength) {
    const char str[6] = "abcde"; // 5 chars + null terminator
    char buffer[64] = {};
    size_t written = SerializeAttribute<const char *>(str, buffer, 5, '\n');
    EXPECT_EQ(written, 6);
    EXPECT_EQ(std::string(buffer, 5), "abcde");
    EXPECT_EQ(buffer[5], '\n');
}

TEST(SerializeAttributeTest, FixedStringWithNullPadding) {
    char str[10] = {};
    std::memcpy(str, "hi", 2);
    char buffer[64] = {};
    size_t written = SerializeAttribute<const char *>(str, buffer, 10, ',');
    EXPECT_EQ(written, 3);
    EXPECT_EQ(std::string(buffer, 2), "hi");
    EXPECT_EQ(buffer[2], ',');
}

// =============================================
// Full tuple serialization
// =============================================

TEST(SerializeAttributeTest, FullTupleSerialization) {
    char buffer[256] = {};
    size_t offset = 0;

    int intVal = 42;
    offset += SerializeAttribute<int>(&intVal, buffer + offset, 0, ',');

    double dblVal = 3.14;
    offset += SerializeAttribute<double>(&dblVal, buffer + offset, 0, ',');

    char charVal = 'X';
    offset += SerializeAttribute<char>(&charVal, buffer + offset, 0, '\n');

    EXPECT_EQ(std::string(buffer, offset), "42,3.14,X\n");
}

TEST(SerializeAttributeTest, TupleWithString) {
    char buffer[256] = {};
    size_t offset = 0;

    int orderKey = 1001;
    offset += SerializeAttribute<int>(&orderKey, buffer + offset, 0, ',');

    double price = 99.99;
    offset += SerializeAttribute<double>(&price, buffer + offset, 0, ',');

    const char name[25] = "Widget";
    offset += SerializeAttribute<const char *>(name, buffer + offset, 25, '\n');

    EXPECT_EQ(std::string(buffer, offset), "1001,99.99,Widget\n");
}

// =============================================
// RuntimeEnv tests
// =============================================

TEST(RuntimeEnvTest, CalculateTupleSize) {
    xdbc::RuntimeEnv env;
    env.buffer_size = 64; // 64 KB
    env.schema = {
        {"l_orderkey", "INT", 4},
        {"l_quantity", "DOUBLE", 8},
        {"l_returnflag", "CHAR", 1},
        {"l_comment", "STRING", 25}
    };

    env.calculateTupleSize();

    EXPECT_EQ(env.tuple_size, 4 + 8 + 1 + 25);
    EXPECT_EQ(env.tuples_per_buffer, (64 * 1024) / 38);
}

TEST(RuntimeEnvTest, CalculateTupleSizeSingleAttribute) {
    xdbc::RuntimeEnv env;
    env.buffer_size = 1; // 1 KB
    env.schema = {{"id", "INT", 4}};

    env.calculateTupleSize();

    EXPECT_EQ(env.tuple_size, 4);
    EXPECT_EQ(env.tuples_per_buffer, 1024 / 4);
}

TEST(RuntimeEnvTest, ToStringContainsFields) {
    xdbc::RuntimeEnv env;
    env.env_name = "test_env";
    env.table = "lineitem";
    env.buffer_size = 64;
    env.buffers_in_bufferpool = 100;

    std::string s = env.toString();
    EXPECT_NE(s.find("test_env"), std::string::npos);
    EXPECT_NE(s.find("lineitem"), std::string::npos);
}
