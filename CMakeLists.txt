cmake_minimum_required(VERSION 3.18)
project(semestralka C)

set(CMAKE_C_STANDARD 11)

add_executable(semestralka main.c)
add_executable(client client.c)
target_link_libraries(semestralka -pthread)
target_link_libraries(client -pthread)

find_package(OpenSSL REQUIRED)
target_link_libraries(client OpenSSL::SSL)