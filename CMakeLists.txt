cmake_minimum_required(VERSION 3.16)
project(spo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-pthread")

find_package (bson-1.0 1.7 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(JSONC REQUIRED IMPORTED_TARGET json-c)

add_executable(client client/client.c util.c mt.h client/cmd_parser.c client/cmd_parser.h)
add_executable(server server/server.c util.c mt.h server/command_api.c server/command_api.h)

find_package(LibXml2 REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/<my_stuff> ${LIBXML2_INCLUDE_DIR})
target_link_libraries(server PRIVATE ${LIBXML2_LIBRARIES})


target_link_libraries(client PRIVATE PkgConfig::JSONC)
target_link_libraries(server PRIVATE PkgConfig::JSONC)
target_link_libraries(server PRIVATE mongo::bson_shared)
target_link_libraries(client PRIVATE mongo::bson_shared)