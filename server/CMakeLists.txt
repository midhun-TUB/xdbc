# Server-only local configurations, dependencies have been moved to root
message(STATUS "Configuring xdbc-server")


#find_package(Arrow REQUIRED)
#find_package(Parquet REQUIRED)
#include_directories(${ARROW_INCLUDE_DIRS} ${PARQUET_INCLUDE_DIRS})

find_library(PARQUET_SHARED_LIB parquet REQUIRED)
# Ensure the library is found
if (NOT PARQUET_SHARED_LIB)
    message(FATAL_ERROR "Parquet shared library not found.")
endif()


find_path(CITYHASH_INCLUDE_DIR NAMES city.h)
find_library(CITYHASH_LIBRARIES NAMES cityhash)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        CITYHASH DEFAULT_MSG
        CITYHASH_LIBRARIES CITYHASH_INCLUDE_DIR)

if (NOT CITYHASH_FOUND)
    message(STATUS "Using third-party bundled CityHash")
else ()
    message(STATUS "Found CITYHASH: ${CITYHASH_LIBRARIES}")
endif (NOT CITYHASH_FOUND)

mark_as_advanced(CITYHASH_INCLUDE_DIR CITYHASH_LIBRARIES)

find_package(Boost 1.40.0 REQUIRED system program_options)

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF (Boost_FOUND)
SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_REGEX_LIBRARY} ${ZSTD_LIBRARY_PATH} ${SNAPPY_LIBRARY_PATH})

#add_definitions(-DUSE_SYSTEM_ABSEIL=OFF)
add_executable(xdbc-server main.cpp xdbcserver.cpp Compression/Compressor.cpp DataSources/PGReader/PGReader.cpp DataSources/CHReader/CHReader.cpp DataSources/CSVReader/CSVReader.cpp DataSources/PQReader/PQReader.cpp DataSources/DataSource.cpp)
#add_library(clickhouse-cpp SHARED IMPORTED)
#set_property(TARGET clickhouse-cpp PROPERTY IMPORTED_LOCATION "/usr/local/lib/libclickhouse-cpp-lib.so")
target_link_libraries(xdbc-server PRIVATE clickhouse-cpp-lib -lpqxx -lpq ${USED_LIBS} lzo2 lz4 cityhash spdlog::spdlog ZLIB::ZLIB FastPFOR::FastPFOR ${FPZIP_LIBRARY} ${ARROW_LIBRARIES} -larrow ${PARQUET_SHARED_LIB})

add_subdirectory(tests)