#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include "../DataSources/deserializers.h"

// =============================================
// Tests for deserialize<int>
// =============================================

TEST(DeserializerTest, IntPositive) {
    const char *src = "12345";
    const char *end = src + strlen(src);
    int result = 0;
    deserialize<int>(src, end, &result, sizeof(int), strlen(src));
    EXPECT_EQ(result, 12345);
}

TEST(DeserializerTest, IntNegative) {
    const char *src = "-42";
    const char *end = src + strlen(src);
    int result = 0;
    deserialize<int>(src, end, &result, sizeof(int), strlen(src));
    EXPECT_EQ(result, -42);
}

TEST(DeserializerTest, IntZero) {
    const char *src = "0";
    const char *end = src + strlen(src);
    int result = -1;
    deserialize<int>(src, end, &result, sizeof(int), strlen(src));
    EXPECT_EQ(result, 0);
}

TEST(DeserializerTest, IntLargeValue) {
    const char *src = "2147483647"; // INT_MAX
    const char *end = src + strlen(src);
    int result = 0;
    deserialize<int>(src, end, &result, sizeof(int), strlen(src));
    EXPECT_EQ(result, 2147483647);
}

// =============================================
// Tests for deserialize<double>
// =============================================

TEST(DeserializerTest, DoublePositive) {
    const char *src = "3.14";
    const char *end = src + strlen(src);
    double result = 0.0;
    deserialize<double>(src, end, &result, sizeof(double), strlen(src));
    EXPECT_DOUBLE_EQ(result, 3.14);
}

TEST(DeserializerTest, DoubleNegative) {
    const char *src = "-99.99";
    const char *end = src + strlen(src);
    double result = 0.0;
    deserialize<double>(src, end, &result, sizeof(double), strlen(src));
    EXPECT_DOUBLE_EQ(result, -99.99);
}

TEST(DeserializerTest, DoubleZero) {
    const char *src = "0.0";
    const char *end = src + strlen(src);
    double result = -1.0;
    deserialize<double>(src, end, &result, sizeof(double), strlen(src));
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST(DeserializerTest, DoubleLargeValue) {
    const char *src = "123456789.123";
    const char *end = src + strlen(src);
    double result = 0.0;
    deserialize<double>(src, end, &result, sizeof(double), strlen(src));
    EXPECT_NEAR(result, 123456789.123, 0.001);
}

TEST(DeserializerTest, DoubleWholeNumber) {
    const char *src = "100";
    const char *end = src + strlen(src);
    double result = 0.0;
    deserialize<double>(src, end, &result, sizeof(double), strlen(src));
    EXPECT_DOUBLE_EQ(result, 100.0);
}

// =============================================
// Tests for deserialize<char>
// =============================================

TEST(DeserializerTest, CharSingle) {
    const char *src = "A";
    const char *end = src + 1;
    char result = '\0';
    deserialize<char>(src, end, &result, sizeof(char), 1);
    EXPECT_EQ(result, 'A');
}

TEST(DeserializerTest, CharDigit) {
    const char *src = "9";
    const char *end = src + 1;
    char result = '\0';
    deserialize<char>(src, end, &result, sizeof(char), 1);
    EXPECT_EQ(result, '9');
}

// =============================================
// Tests for deserialize<const char *> (fixed-size string)
// =============================================

TEST(DeserializerTest, StringShort) {
    const char *src = "hello";
    const char *end = src + 5;
    char dest[10] = {};
    memset(dest, 'X', 10); // fill with sentinel

    deserialize<const char *>(src, end, dest, 10, 5);

    // First 5 bytes should be "hello", rest should be zeroed
    EXPECT_EQ(std::string(dest, 5), "hello");
    // Bytes 5-9 should be zero (memset to 0 in the deserializer)
    for (int i = 5; i < 10; i++) {
        EXPECT_EQ(dest[i], '\0');
    }
}

TEST(DeserializerTest, StringExactLength) {
    const char *src = "abcde";
    const char *end = src + 5;
    char dest[5] = {};

    deserialize<const char *>(src, end, dest, 5, 5);
    EXPECT_EQ(std::string(dest, 5), "abcde");
}

TEST(DeserializerTest, StringEmpty) {
    const char *src = "";
    const char *end = src;
    char dest[10] = {};
    memset(dest, 'X', 10);

    deserialize<const char *>(src, end, dest, 10, 0);

    // All bytes should be zeroed
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(dest[i], '\0');
    }
}

// =============================================
// Simulated CSV row deserialization
// =============================================

TEST(DeserializerTest, FullCSVRow) {
    // Simulate parsing: "42,3.14,A,hello\n"
    // Schema: INT(4), DOUBLE(8), CHAR(1), STRING(10)
    const int tuple_size = 4 + 8 + 1 + 10; // 23
    char tuple_buffer[23] = {};

    // Parse INT field "42"
    {
        const char *src = "42";
        const char *end = src + 2;
        deserialize<int>(src, end, tuple_buffer + 0, 4, 2);
    }

    // Parse DOUBLE field "3.14"
    {
        const char *src = "3.14";
        const char *end = src + 4;
        deserialize<double>(src, end, tuple_buffer + 4, 8, 4);
    }

    // Parse CHAR field "A"
    {
        const char *src = "A";
        const char *end = src + 1;
        deserialize<char>(src, end, tuple_buffer + 12, 1, 1);
    }

    // Parse STRING field "hello"
    {
        const char *src = "hello";
        const char *end = src + 5;
        deserialize<const char *>(src, end, tuple_buffer + 13, 10, 5);
    }

    // Verify
    int intVal = *reinterpret_cast<int *>(tuple_buffer);
    double dblVal = *reinterpret_cast<double *>(tuple_buffer + 4);
    char charVal = tuple_buffer[12];
    std::string strVal(tuple_buffer + 13, 5);

    EXPECT_EQ(intVal, 42);
    EXPECT_DOUBLE_EQ(dblVal, 3.14);
    EXPECT_EQ(charVal, 'A');
    EXPECT_EQ(strVal, "hello");
}
