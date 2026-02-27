#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include <random>
#include <numeric>

#include <zstd.h>
#include <snappy.h>
#include <lzo/lzo1x.h>
#include <lz4.h>
#include <zlib.h>

#include "../../xdbc/Decompression/Decompressor.h"

class DecompressorTest : public ::testing::Test {
protected:
    void SetUp() override {
        lzo_init();
    }

    std::vector<char> generateCompressibleData(size_t size) {
        std::vector<char> data(size);
        const std::string pattern = "Hello XDBC World! This is a repeating pattern for decompression testing. ";
        for (size_t i = 0; i < size; i++) {
            data[i] = pattern[i % pattern.size()];
        }
        return data;
    }
};

// --- ZSTD Decompress ---

TEST_F(DecompressorTest, ZstdDecompress) {
    auto original = generateCompressibleData(64 * 1024);

    // Compress with ZSTD
    size_t maxCompSize = ZSTD_compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    size_t compSize = ZSTD_compress(compressed.data(), maxCompSize,
                                     original.data(), original.size(), 1);
    ASSERT_FALSE(ZSTD_isError(compSize));

    // Decompress with our Decompressor
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress_zstd(decompressed.data(), compressed.data(),
                                             compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

// --- Snappy Decompress ---

TEST_F(DecompressorTest, SnappyDecompress) {
    auto original = generateCompressibleData(64 * 1024);

    // Compress with Snappy
    std::string compressed;
    snappy::Compress(original.data(), original.size(), &compressed);

    // Decompress with our Decompressor
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress_snappy(decompressed.data(), compressed.data(),
                                               compressed.size(), original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

// --- LZO Decompress ---

TEST_F(DecompressorTest, LzoDecompress) {
    auto original = generateCompressibleData(64 * 1024);

    // Compress with LZO
    std::vector<unsigned char> compressed(original.size() + original.size() / 16 + 64 + 3);
    lzo_uint compSize;
    std::vector<unsigned char> wrkmem(LZO1X_1_MEM_COMPRESS);
    int rc = lzo1x_1_compress(reinterpret_cast<const unsigned char *>(original.data()),
                              original.size(), compressed.data(), &compSize, wrkmem.data());
    ASSERT_EQ(rc, LZO_E_OK);

    // Decompress with our Decompressor
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress_lzo(decompressed.data(), compressed.data(),
                                            compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

// --- LZ4 Decompress ---

TEST_F(DecompressorTest, Lz4Decompress) {
    auto original = generateCompressibleData(64 * 1024);

    // Compress with LZ4
    int maxCompSize = LZ4_compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    int compSize = LZ4_compress_default(original.data(), compressed.data(),
                                         original.size(), maxCompSize);
    ASSERT_GT(compSize, 0);

    // Decompress with our Decompressor
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress_lz4(decompressed.data(), compressed.data(),
                                            compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

// --- ZLIB Decompress ---

TEST_F(DecompressorTest, ZlibDecompress) {
    auto original = generateCompressibleData(64 * 1024);

    // Compress with zlib
    uLongf maxCompSize = compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    uLongf compSize = maxCompSize;
    int rc = compress2(reinterpret_cast<Bytef *>(compressed.data()), &compSize,
                       reinterpret_cast<const Bytef *>(original.data()), original.size(), 9);
    ASSERT_EQ(rc, Z_OK);

    // Decompress with our Decompressor
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress_zlib(decompressed.data(), compressed.data(),
                                             compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

// --- Dispatch method tests ---

TEST_F(DecompressorTest, DispatchZstd) {
    auto original = generateCompressibleData(32 * 1024);
    size_t maxCompSize = ZSTD_compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    size_t compSize = ZSTD_compress(compressed.data(), maxCompSize,
                                     original.data(), original.size(), 1);
    ASSERT_FALSE(ZSTD_isError(compSize));

    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress(1, decompressed.data(), compressed.data(),
                                        compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

TEST_F(DecompressorTest, DispatchSnappy) {
    auto original = generateCompressibleData(32 * 1024);
    std::string compressed;
    snappy::Compress(original.data(), original.size(), &compressed);

    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress(2, decompressed.data(), compressed.data(),
                                        compressed.size(), original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

TEST_F(DecompressorTest, DispatchLz4) {
    auto original = generateCompressibleData(32 * 1024);
    int maxCompSize = LZ4_compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    int compSize = LZ4_compress_default(original.data(), compressed.data(),
                                         original.size(), maxCompSize);
    ASSERT_GT(compSize, 0);

    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress(4, decompressed.data(), compressed.data(),
                                        compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

TEST_F(DecompressorTest, DispatchUnknownReturns1) {
    char data[16] = {};
    char out[16] = {};
    // method 0 and unknown methods return 1 (passthrough)
    int ret = Decompressor::decompress(0, out, data, 16, 16);
    EXPECT_EQ(ret, 1);

    ret = Decompressor::decompress(99, out, data, 16, 16);
    EXPECT_EQ(ret, 1);
}

// --- Cross-library round-trip (simulates server compress -> client decompress) ---

TEST_F(DecompressorTest, RoundTripZstdServerToClient) {
    auto original = generateCompressibleData(128 * 1024);

    // "Server" compresses
    size_t maxCompSize = ZSTD_compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    size_t compSize = ZSTD_compress(compressed.data(), maxCompSize,
                                     original.data(), original.size(), 1);
    ASSERT_FALSE(ZSTD_isError(compSize));
    ASSERT_LT(compSize, original.size());

    // "Client" decompresses
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress(1, decompressed.data(), compressed.data(),
                                        compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

TEST_F(DecompressorTest, RoundTripLz4ServerToClient) {
    auto original = generateCompressibleData(128 * 1024);

    // "Server" compresses
    int maxCompSize = LZ4_compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    int compSize = LZ4_compress_default(original.data(), compressed.data(),
                                         original.size(), maxCompSize);
    ASSERT_GT(compSize, 0);

    // "Client" decompresses
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress(4, decompressed.data(), compressed.data(),
                                        compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

TEST_F(DecompressorTest, RoundTripZlibServerToClient) {
    auto original = generateCompressibleData(128 * 1024);

    // "Server" compresses
    uLongf maxCompSize = compressBound(original.size());
    std::vector<char> compressed(maxCompSize);
    uLongf compSize = maxCompSize;
    int rc = compress2(reinterpret_cast<Bytef *>(compressed.data()), &compSize,
                       reinterpret_cast<const Bytef *>(original.data()), original.size(), 9);
    ASSERT_EQ(rc, Z_OK);

    // "Client" decompresses
    std::vector<char> decompressed(original.size());
    int ret = Decompressor::decompress(5, decompressed.data(), compressed.data(),
                                        compSize, original.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(original.data(), decompressed.data(), original.size()), 0);
}

// --- Integer/Double column data round-trip ---

TEST_F(DecompressorTest, RoundTripIntegerColumnData) {
    const size_t numInts = 8192;
    std::vector<int32_t> original(numInts);
    std::iota(original.begin(), original.end(), 100);

    size_t byteSize = numInts * sizeof(int32_t);
    size_t maxCompSize = ZSTD_compressBound(byteSize);
    std::vector<char> compressed(maxCompSize);

    size_t compSize = ZSTD_compress(compressed.data(), maxCompSize,
                                     original.data(), byteSize, 1);
    ASSERT_FALSE(ZSTD_isError(compSize));

    std::vector<int32_t> decompressed(numInts);
    int ret = Decompressor::decompress(1, decompressed.data(), compressed.data(),
                                        compSize, byteSize);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(original, decompressed);
}

TEST_F(DecompressorTest, RoundTripDoubleColumnData) {
    const size_t numDoubles = 4096;
    std::vector<double> original(numDoubles);
    for (size_t i = 0; i < numDoubles; i++) {
        original[i] = static_cast<double>(i) * 1.5;
    }

    size_t byteSize = numDoubles * sizeof(double);
    int maxCompSize = LZ4_compressBound(byteSize);
    std::vector<char> compressed(maxCompSize);

    int compSize = LZ4_compress_default(reinterpret_cast<const char *>(original.data()),
                                         compressed.data(), byteSize, maxCompSize);
    ASSERT_GT(compSize, 0);

    std::vector<double> decompressed(numDoubles);
    int ret = Decompressor::decompress(4, decompressed.data(), compressed.data(),
                                        compSize, byteSize);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(original, decompressed);
}
