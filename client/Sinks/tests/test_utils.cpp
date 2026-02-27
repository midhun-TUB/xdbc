#include <gtest/gtest.h>
#include <vector>
#include <atomic>
#include <cstdint>
#include <boost/asio.hpp>
#include "../../xdbc/utils.h"

// =============================================
// Tests for Utils::compute_checksum
// =============================================

TEST(UtilsTest, ChecksumAllZeros) {
    uint8_t data[8] = {};
    EXPECT_EQ(Utils::compute_checksum(data, 8), 0);
}

TEST(UtilsTest, ChecksumSingleByte) {
    uint8_t data[1] = {0xAB};
    EXPECT_EQ(Utils::compute_checksum(data, 1), 0xAB);
}

TEST(UtilsTest, ChecksumXorProperty) {
    // XOR of same values cancels out
    uint8_t data[2] = {0xFF, 0xFF};
    EXPECT_EQ(Utils::compute_checksum(data, 2), 0);
}

TEST(UtilsTest, ChecksumKnownValue) {
    uint8_t data[4] = {0x01, 0x02, 0x04, 0x08};
    // 0x01 ^ 0x02 = 0x03, 0x03 ^ 0x04 = 0x07, 0x07 ^ 0x08 = 0x0F
    EXPECT_EQ(Utils::compute_checksum(data, 4), 0x0F);
}

// =============================================
// Tests for Utils::boolVectorToString
// =============================================

TEST(UtilsTest, BoolVectorToStringAllTrue) {
    std::vector<std::atomic<bool>> vec(3);
    vec[0] = true;
    vec[1] = true;
    vec[2] = true;
    EXPECT_EQ(Utils::boolVectorToString(vec), "1,1,1");
}

TEST(UtilsTest, BoolVectorToStringAllFalse) {
    std::vector<std::atomic<bool>> vec(3);
    vec[0] = false;
    vec[1] = false;
    vec[2] = false;
    EXPECT_EQ(Utils::boolVectorToString(vec), "0,0,0");
}

TEST(UtilsTest, BoolVectorToStringMixed) {
    std::vector<std::atomic<bool>> vec(4);
    vec[0] = true;
    vec[1] = false;
    vec[2] = true;
    vec[3] = false;
    EXPECT_EQ(Utils::boolVectorToString(vec), "1,0,1,0");
}

TEST(UtilsTest, BoolVectorToStringSingle) {
    std::vector<std::atomic<bool>> vec(1);
    vec[0] = true;
    EXPECT_EQ(Utils::boolVectorToString(vec), "1");
}

// =============================================
// Tests for Utils::boolVecAsStr (same logic, different function)
// =============================================

TEST(UtilsTest, BoolVecAsStrMixed) {
    std::vector<std::atomic<bool>> vec(3);
    vec[0] = false;
    vec[1] = true;
    vec[2] = false;
    EXPECT_EQ(Utils::boolVecAsStr(vec), "0,1,0");
}

// =============================================
// Tests for Utils::slStr (shortLineitem formatting)
// =============================================

TEST(UtilsTest, ShortLineitemString) {
    Utils::shortLineitem sl;
    sl.l_orderkey = 1;
    sl.l_partkey = 2;
    sl.l_suppkey = 3;
    sl.l_linenumber = 4;
    sl.l_quantity = 5.0;
    sl.l_extendedprice = 6.0;
    sl.l_discount = 7.0;
    sl.l_tax = 8.0;

    std::string result = Utils::slStr(&sl);
    EXPECT_NE(result.find("1"), std::string::npos);
    EXPECT_NE(result.find("2"), std::string::npos);
    EXPECT_NE(result.find("3"), std::string::npos);
    EXPECT_NE(result.find("4"), std::string::npos);
}
