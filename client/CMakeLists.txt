# Client-only local configurations, dependencies have been moved to root

message(STATUS "Configuring xdbc-client")

add_library(xdbc-client-lib SHARED
        xdbc/xclient.cpp
        xdbc/Decompression/Decompressor.cpp xdbc/Decompression/Decompressor.h)

set_target_properties(xdbc-client-lib PROPERTIES
        #VERSION ${PROJECT_VERSION}
        #SOVERSION 1
        PUBLIC_HEADER "xdbc/xclient.h;xdbc/SinkInterface.h")

target_include_directories(xdbc-client-lib PUBLIC .)
target_link_libraries(xdbc-client-lib PRIVATE ${ZSTD_LIBRARY_PATH} ${SNAPPY_LIBRARY_PATH} Threads::Threads lzo2 lz4 spdlog::spdlog_header_only ZLIB::ZLIB FastPFOR::FastPFOR ${FPZIP_LIBRARY})
install(TARGETS xdbc-client-lib
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES xdbc/customQueue.h xdbc/utils.h xdbc/metrics_calculator.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_subdirectory(Sinks)
