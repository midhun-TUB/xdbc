#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../DataSources/DataSource.h"

// Test the free functions and static methods from DataSource.cpp

// These are defined in DataSource.cpp but not declared in the header.
// Re-declare them here for testing.
SchemaAttribute createSchemaAttribute(std::string name, std::string tpe, int size);
std::vector<SchemaAttribute> createSchemaFromJsonString(const std::string &jsonString);

// =============================================
// createSchemaAttribute tests
// =============================================

TEST(DataSourceUtilTest, CreateSchemaAttribute) {
    auto attr = createSchemaAttribute("l_orderkey", "INT", 4);
    EXPECT_EQ(attr.name, "l_orderkey");
    EXPECT_EQ(attr.tpe, "INT");
    EXPECT_EQ(attr.size, 4);
}

TEST(DataSourceUtilTest, CreateSchemaAttributeString) {
    auto attr = createSchemaAttribute("l_comment", "STRING", 44);
    EXPECT_EQ(attr.name, "l_comment");
    EXPECT_EQ(attr.tpe, "STRING");
    EXPECT_EQ(attr.size, 44);
}

// =============================================
// createSchemaFromJsonString tests
// =============================================

TEST(DataSourceUtilTest, CreateSchemaFromJsonSingleAttr) {
    std::string json = R"([{"name":"id","type":"INT","size":4}])";
    auto schema = createSchemaFromJsonString(json);

    ASSERT_EQ(schema.size(), 1);
    EXPECT_EQ(schema[0].name, "id");
    EXPECT_EQ(schema[0].tpe, "INT");
    EXPECT_EQ(schema[0].size, 4);
}

TEST(DataSourceUtilTest, CreateSchemaFromJsonMultipleAttrs) {
    std::string json = R"([
        {"name":"l_orderkey","type":"INT","size":4},
        {"name":"l_quantity","type":"DOUBLE","size":8},
        {"name":"l_returnflag","type":"CHAR","size":1},
        {"name":"l_comment","type":"STRING","size":44}
    ])";
    auto schema = createSchemaFromJsonString(json);

    ASSERT_EQ(schema.size(), 4);
    EXPECT_EQ(schema[0].name, "l_orderkey");
    EXPECT_EQ(schema[0].tpe, "INT");
    EXPECT_EQ(schema[0].size, 4);

    EXPECT_EQ(schema[1].name, "l_quantity");
    EXPECT_EQ(schema[1].tpe, "DOUBLE");
    EXPECT_EQ(schema[1].size, 8);

    EXPECT_EQ(schema[2].name, "l_returnflag");
    EXPECT_EQ(schema[2].tpe, "CHAR");
    EXPECT_EQ(schema[2].size, 1);

    EXPECT_EQ(schema[3].name, "l_comment");
    EXPECT_EQ(schema[3].tpe, "STRING");
    EXPECT_EQ(schema[3].size, 44);
}

TEST(DataSourceUtilTest, CreateSchemaFromJsonEmpty) {
    std::string json = "[]";
    auto schema = createSchemaFromJsonString(json);
    EXPECT_TRUE(schema.empty());
}

TEST(DataSourceUtilTest, CreateSchemaFromJsonInvalidThrows) {
    std::string json = "not valid json";
    EXPECT_THROW(createSchemaFromJsonString(json), std::exception);
}

// =============================================
// DataSource::getSchemaSize tests
// =============================================

TEST(DataSourceUtilTest, GetSchemaSizeSingle) {
    std::vector<SchemaAttribute> schema = {{"id", "INT", 4}};
    EXPECT_EQ(DataSource::getSchemaSize(schema), 4);
}

TEST(DataSourceUtilTest, GetSchemaSizeMixed) {
    std::vector<SchemaAttribute> schema = {
        {"l_orderkey", "INT", 4},
        {"l_quantity", "DOUBLE", 8},
        {"l_returnflag", "CHAR", 1},
        {"l_comment", "STRING", 44}
    };
    EXPECT_EQ(DataSource::getSchemaSize(schema), 4 + 8 + 1 + 44);
}

TEST(DataSourceUtilTest, GetSchemaSizeEmpty) {
    std::vector<SchemaAttribute> schema;
    EXPECT_EQ(DataSource::getSchemaSize(schema), 0);
}
