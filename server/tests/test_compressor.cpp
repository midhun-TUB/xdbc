#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include <random>
#include <string>
#include <numeric>

#include <zstd.h>
#include <snappy.h>
#include <lzo/lzo1x.h>
#include <lz4.h>
#include <zlib.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// Re-implement the static compression functions from Compressor.cpp
// to avoid pulling in the full xdbcserver.h dependency chain.
// These match the logic in server/Compression/Compressor.cpp exactly.

namespace comp {

size_t compress_zstd(void *data, void *dst, size_t size) {
    size_t compressedSize = ZSTD_compress(dst, size, data, size, 1);
    if (ZSTD_isError(compressedSize)) {
        return size;
    }
    return compressedSize;
}

size_t compress_snappy(void *data, void *dst, size_t size) {
    size_t compressedSize;
    snappy::RawCompress(static_cast<const char *>(data), size, static_cast<char *>(dst), &compressedSize);
    if (compressedSize >= size) {
        return size;
    }
    return compressedSize;
}

size_t compress_lzo(void *src, void *dst, size_t size) {
    lzo_voidp wrkmem = (lzo_voidp) malloc(LZO1X_1_MEM_COMPRESS);
    if (!wrkmem) {
        return size;
    }

    lzo_uint compressedSize;
    int result = lzo1x_1_compress(static_cast<const unsigned char *>(src), size,
                                  static_cast<unsigned char *>(dst), &compressedSize, wrkmem);
    free(wrkmem);

    if (result != LZO_E_OK || compressedSize > size) {
        return size;
    }
    return compressedSize;
}

size_t compress_lz4(void *src, void *dst, size_t size) {
    int compressedSize = LZ4_compress_default(static_cast<const char *>(src), static_cast<char *>(dst), size, size);
    if (compressedSize <= 0 || compressedSize >= static_cast<int>(size)) {
        return size;
    }
    return compressedSize;
}

size_t compress_zlib(void *src, void *dst, size_t size) {
    uLongf maxCompressedSize = compressBound(size);
    uLongf compressedSize = maxCompressedSize;
    int compression_level = 9;

    // dst must have enough space for zlib output
    int result = compress2(static_cast<Bytef *>(dst), &compressedSize,
                           static_cast<const Bytef *>(src), size, compression_level);
    if (result != Z_OK || compressedSize >= size) {
        return size;
    }
    return compressedSize;
}

size_t getCompId(const std::string &name) {
    if (name == "nocomp") return 0;
    else if (name == "zstd") return 1;
    else if (name == "snappy") return 2;
    else if (name == "lzo") return 3;
    else if (name == "lz4") return 4;
    else if (name == "zlib") return 5;
    else if (name == "cols") return 6;
    return 0;
}

} // namespace comp

class CompressorTest : public ::testing::Test {
protected:
    void SetUp() override {
        lzo_init();
    }

    // Generate compressible data (repeated pattern)
    std::vector<char> generateCompressibleData(size_t size) {
        std::vector<char> data(size);
        const std::string pattern = "Hello XDBC World! This is a repeating pattern for compression testing. ";
        for (size_t i = 0; i < size; i++) {
            data[i] = pattern[i % pattern.size()];
        }
        return data;
    }

    // Generate random (incompressible) data
    std::vector<char> generateRandomData(size_t size) {
        std::vector<char> data(size);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, 255);
        for (size_t i = 0; i < size; i++) {
            data[i] = static_cast<char>(dist(rng));
        }
        return data;
    }
};

// --- getCompId tests ---

TEST_F(CompressorTest, GetCompIdMapping) {
    EXPECT_EQ(comp::getCompId("nocomp"), 0);
    EXPECT_EQ(comp::getCompId("zstd"), 1);
    EXPECT_EQ(comp::getCompId("snappy"), 2);
    EXPECT_EQ(comp::getCompId("lzo"), 3);
    EXPECT_EQ(comp::getCompId("lz4"), 4);
    EXPECT_EQ(comp::getCompId("zlib"), 5);
    EXPECT_EQ(comp::getCompId("cols"), 6);
    EXPECT_EQ(comp::getCompId("unknown"), 0);
    EXPECT_EQ(comp::getCompId(""), 0);
}

// --- ZSTD Tests ---

TEST_F(CompressorTest, ZstdCompressesData) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(data.size());

    size_t compressedSize = comp::compress_zstd(data.data(), compressed.data(), data.size());
    EXPECT_GT(compressedSize, 0);
    EXPECT_LT(compressedSize, data.size());
}

TEST_F(CompressorTest, ZstdRoundTrip) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(data.size());

    size_t compressedSize = comp::compress_zstd(data.data(), compressed.data(), data.size());
    ASSERT_LT(compressedSize, data.size());

    std::vector<char> decompressed(data.size());
    size_t decompSize = ZSTD_decompress(decompressed.data(), decompressed.size(),
                                         compressed.data(), compressedSize);
    ASSERT_FALSE(ZSTD_isError(decompSize));
    EXPECT_EQ(decompSize, data.size());
    EXPECT_EQ(memcmp(data.data(), decompressed.data(), data.size()), 0);
}

// --- Snappy Tests ---

TEST_F(CompressorTest, SnappyCompressesData) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(snappy::MaxCompressedLength(data.size()));

    size_t compressedSize = comp::compress_snappy(data.data(), compressed.data(), data.size());
    EXPECT_GT(compressedSize, 0);
    EXPECT_LT(compressedSize, data.size());
}

TEST_F(CompressorTest, SnappyRoundTrip) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(snappy::MaxCompressedLength(data.size()));

    size_t compressedSize = comp::compress_snappy(data.data(), compressed.data(), data.size());
    ASSERT_LT(compressedSize, data.size());

    std::string decompressed;
    ASSERT_TRUE(snappy::Uncompress(compressed.data(), compressedSize, &decompressed));
    EXPECT_EQ(decompressed.size(), data.size());
    EXPECT_EQ(memcmp(data.data(), decompressed.data(), data.size()), 0);
}

// --- LZO Tests ---

TEST_F(CompressorTest, LzoCompressesData) {
    auto data = generateCompressibleData(64 * 1024);
    // LZO needs extra buffer space
    std::vector<char> compressed(data.size() + data.size() / 16 + 64 + 3);

    size_t compressedSize = comp::compress_lzo(data.data(), compressed.data(), data.size());
    EXPECT_GT(compressedSize, 0);
    EXPECT_LT(compressedSize, data.size());
}

TEST_F(CompressorTest, LzoRoundTrip) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(data.size() + data.size() / 16 + 64 + 3);

    size_t compressedSize = comp::compress_lzo(data.data(), compressed.data(), data.size());
    ASSERT_LT(compressedSize, data.size());

    std::vector<unsigned char> decompressed(data.size());
    lzo_uint decompSize = decompressed.size();
    int rc = lzo1x_decompress(reinterpret_cast<const unsigned char *>(compressed.data()),
                              compressedSize,
                              decompressed.data(), &decompSize, nullptr);
    ASSERT_EQ(rc, LZO_E_OK);
    EXPECT_EQ(decompSize, data.size());
    EXPECT_EQ(memcmp(data.data(), decompressed.data(), data.size()), 0);
}

// --- LZ4 Tests ---

TEST_F(CompressorTest, Lz4CompressesData) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(LZ4_compressBound(data.size()));

    size_t compressedSize = comp::compress_lz4(data.data(), compressed.data(), data.size());
    EXPECT_GT(compressedSize, 0);
    EXPECT_LT(compressedSize, data.size());
}

TEST_F(CompressorTest, Lz4RoundTrip) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(LZ4_compressBound(data.size()));

    size_t compressedSize = comp::compress_lz4(data.data(), compressed.data(), data.size());
    ASSERT_LT(compressedSize, data.size());

    std::vector<char> decompressed(data.size());
    int decompSize = LZ4_decompress_safe(compressed.data(), decompressed.data(),
                                          compressedSize, decompressed.size());
    ASSERT_GT(decompSize, 0);
    EXPECT_EQ(static_cast<size_t>(decompSize), data.size());
    EXPECT_EQ(memcmp(data.data(), decompressed.data(), data.size()), 0);
}

// --- ZLIB Tests ---

TEST_F(CompressorTest, ZlibCompressesData) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(compressBound(data.size()));

    size_t compressedSize = comp::compress_zlib(data.data(), compressed.data(), data.size());
    EXPECT_GT(compressedSize, 0);
    EXPECT_LT(compressedSize, data.size());
}

TEST_F(CompressorTest, ZlibRoundTrip) {
    auto data = generateCompressibleData(64 * 1024);
    std::vector<char> compressed(compressBound(data.size()));

    size_t compressedSize = comp::compress_zlib(data.data(), compressed.data(), data.size());
    ASSERT_LT(compressedSize, data.size());

    std::vector<char> decompressed(data.size());
    uLongf decompSize = decompressed.size();
    int rc = uncompress(reinterpret_cast<Bytef *>(decompressed.data()), &decompSize,
                        reinterpret_cast<const Bytef *>(compressed.data()), compressedSize);
    ASSERT_EQ(rc, Z_OK);
    EXPECT_EQ(decompSize, data.size());
    EXPECT_EQ(memcmp(data.data(), decompressed.data(), data.size()), 0);
}

// --- Small Data Tests ---

TEST_F(CompressorTest, ZstdSmallData) {
    const char small[] = "tiny";
    char compressed[256];
    size_t result = comp::compress_zstd(const_cast<char *>(small), compressed, sizeof(small));
    // Small data may not compress well, function should still succeed
    EXPECT_GT(result, 0);
}

TEST_F(CompressorTest, Lz4SmallData) {
    const char small[] = "tiny";
    char compressed[256];
    // LZ4 with dst_size == src_size may fail for tiny data, returning original size
    size_t result = comp::compress_lz4(const_cast<char *>(small), compressed, sizeof(small));
    EXPECT_GT(result, 0);
}

// --- Various Data Sizes ---

TEST_F(CompressorTest, ZstdVariousSizes) {
    for (size_t size : {1024, 4096, 16384, 65536, 262144}) {
        auto data = generateCompressibleData(size);
        std::vector<char> compressed(data.size());
        size_t compressedSize = comp::compress_zstd(data.data(), compressed.data(), data.size());
        EXPECT_GT(compressedSize, 0) << "Failed for size " << size;
        EXPECT_LT(compressedSize, data.size()) << "No compression for size " << size;
    }
}

// --- Integer Data (simulating column data) ---

TEST_F(CompressorTest, ZstdIntegerData) {
    const size_t numInts = 16384;
    std::vector<int32_t> data(numInts);
    // Sequential integers compress very well
    std::iota(data.begin(), data.end(), 0);

    size_t byteSize = numInts * sizeof(int32_t);
    std::vector<char> compressed(byteSize);

    size_t compressedSize = comp::compress_zstd(data.data(), compressed.data(), byteSize);
    EXPECT_LT(compressedSize, byteSize);

    // Verify round-trip
    std::vector<int32_t> decompressed(numInts);
    size_t decompSize = ZSTD_decompress(decompressed.data(), byteSize,
                                         compressed.data(), compressedSize);
    ASSERT_FALSE(ZSTD_isError(decompSize));
    EXPECT_EQ(data, decompressed);
}
