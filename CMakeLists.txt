cmake_minimum_required(VERSION 3.9)
project(PowServer)

set(CMAKE_CXX_STANDARD 17)

# GRPC and Protocol Buffers libraries location
list(APPEND CMAKE_PREFIX_PATH
        "/opt/grpc"
        "/opt/protobuf"
        "/usr/local/Cellar/grpc/"
        "/usr/local/Cellar/protobuf/"
        "/usr/local/Cellar/openssl/1.0.2n")

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(OpenSSL REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR} proof_of_work_game.proto)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${CMAKE_CURRENT_SOURCE_DIR} proof_of_work_game.proto)

add_library(CryptoHelpers
        crypto_helpers.cc)

add_library(PowClient
        pow_client.cc
        ${PROTO_SRCS}
        ${GRPC_SRCS})

add_executable(PowServer
        pow_server.cc
        ${PROTO_SRCS}
        ${GRPC_SRCS})

add_executable(PowClientMain
        client_main.cc)

target_link_libraries(CryptoHelpers
        OpenSSL::Crypto)

target_link_libraries(PowServer
        gRPC::grpc++_reflection
        protobuf::libprotobuf
        CryptoHelpers)

target_link_libraries(PowClient
        gRPC::grpc++_reflection
        protobuf::libprotobuf
        CryptoHelpers)

target_link_libraries(PowClientMain
        PowClient)