#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <parquet/schema.h>
#include "../../xdbc/RuntimeEnv.h"

// Re-declare CreateParquetSchema from PQSink.cpp to test it.
// It's a free function defined in PQSink.cpp.
std::shared_ptr<parquet::schema::GroupNode>
CreateParquetSchema(const std::vector<xdbc::SchemaAttribute> &schemaAttributes);

TEST(PQSinkTest, CreateSchemaIntOnly) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"id", "INT", 4},
        {"count", "INT", 4}
    };

    auto groupNode = CreateParquetSchema(schema);
    ASSERT_NE(groupNode, nullptr);
    EXPECT_EQ(groupNode->field_count(), 2);
    EXPECT_EQ(groupNode->field(0)->name(), "id");
    EXPECT_EQ(groupNode->field(1)->name(), "count");
}

TEST(PQSinkTest, CreateSchemaDoubleOnly) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"price", "DOUBLE", 8},
        {"quantity", "DOUBLE", 8}
    };

    auto groupNode = CreateParquetSchema(schema);
    ASSERT_NE(groupNode, nullptr);
    EXPECT_EQ(groupNode->field_count(), 2);
}

TEST(PQSinkTest, CreateSchemaStringType) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"name", "STRING", 25}
    };

    auto groupNode = CreateParquetSchema(schema);
    ASSERT_NE(groupNode, nullptr);
    EXPECT_EQ(groupNode->field_count(), 1);
    EXPECT_EQ(groupNode->field(0)->name(), "name");
}

TEST(PQSinkTest, CreateSchemaCharType) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"flag", "CHAR", 1}
    };

    auto groupNode = CreateParquetSchema(schema);
    ASSERT_NE(groupNode, nullptr);
    EXPECT_EQ(groupNode->field_count(), 1);
}

TEST(PQSinkTest, CreateSchemaMixedTypes) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"l_orderkey", "INT", 4},
        {"l_quantity", "DOUBLE", 8},
        {"l_returnflag", "CHAR", 1},
        {"l_comment", "STRING", 44}
    };

    auto groupNode = CreateParquetSchema(schema);
    ASSERT_NE(groupNode, nullptr);
    EXPECT_EQ(groupNode->field_count(), 4);
    EXPECT_EQ(groupNode->field(0)->name(), "l_orderkey");
    EXPECT_EQ(groupNode->field(1)->name(), "l_quantity");
    EXPECT_EQ(groupNode->field(2)->name(), "l_returnflag");
    EXPECT_EQ(groupNode->field(3)->name(), "l_comment");
}

TEST(PQSinkTest, CreateSchemaUnsupportedTypeThrows) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"unknown", "BOOLEAN", 1}
    };

    EXPECT_THROW(CreateParquetSchema(schema), std::invalid_argument);
}

TEST(PQSinkTest, CreateSchemaCharWithZeroSizeThrows) {
    std::vector<xdbc::SchemaAttribute> schema = {
        {"flag", "CHAR", 0}
    };

    EXPECT_THROW(CreateParquetSchema(schema), std::invalid_argument);
}
