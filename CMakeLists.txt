cmake_minimum_required(VERSION 3.16)
project(spo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-pthread")

add_executable(client client/client.c client/cmd_parser.c client/cmd_parser.h util.c util.h mt.h)
add_executable(server server/server.c server/storage.c server/storage.h command_api.c command_api.h util.c util.h)

find_package(LibXml2 REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/<my_stuff> ${LIBXML2_INCLUDE_DIR})

target_link_libraries(server PRIVATE ${LIBXML2_LIBRARIES})
target_link_libraries(client PRIVATE ${LIBXML2_LIBRARIES})