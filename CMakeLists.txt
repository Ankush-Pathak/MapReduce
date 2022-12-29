cmake_minimum_required(VERSION 3.16)
project(Assignment2)
include(FetchContent)


set(CMAKE_CXX_STANDARD 17)
set(Boost_INCLUDE_DIR /usr/include/boost/)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)
#set(spdlog_DIR Thirdparty/spdlog/build)
set(TARGET_LOGGER spdlog)
set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
set(HDFS)
if (CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else ()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif ()


find_package(Boost 1.67.0 COMPONENTS system filesystem serialization REQUIRED)
find_package(Threads REQUIRED)
#if(NOT TARGET spdlog)
#    # Stand-alone build
#    find_package(spdlog REQUIRED)
#endif()

FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG v1.49.1  # e.g v1.28.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

####################################################################################
# GRPC
####################################################################################
# Proto file
get_filename_component(mr_proto "Common/Services/MapReduce.proto" ABSOLUTE)
get_filename_component(mr_proto_path "${mr_proto}" PATH)

# Generated sources
set(mr_proto_srcs "${mr_proto_path}/MapReduce.pb.cc")
set(mr_proto_hdrs "${mr_proto_path}/MapReduce.pb.h")
set(mr_grpc_srcs "${mr_proto_path}/MapReduce.grpc.pb.cc")
set(mr_grpc_hdrs "${mr_proto_path}/MapReduce.grpc.pb.h")
add_custom_command(
        OUTPUT "${mr_proto_srcs}" "${mr_proto_hdrs}" "${mr_grpc_srcs}" "${mr_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${mr_proto_path}"
        --cpp_out "${mr_proto_path}"
        -I "${mr_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${mr_proto}"
        DEPENDS "${mr_proto}")

# Include generated *.pb.h file
#include_directories("${mr_proto_path}")

# rg_grpc_proto
add_library(mr_grpc_proto
        ${mr_grpc_srcs}
        ${mr_grpc_hdrs}
        ${mr_proto_srcs}
        ${mr_proto_hdrs})
target_link_libraries(mr_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

include_directories(. ${Boost_INCLUDE_DIR} ${mr_proto_path})
add_subdirectory(Thirdparty/spdlog)
####################################################################################
# GRPC ends
####################################################################################

#add_executable(Master Master/Master.cpp Master/Master.h Master/MasterConfig/MasterConfig.cpp Master/MasterConfig/MasterConfig.h)
add_executable(HDFS HDFS/server.cpp HDFS/Request/Request.h HDFS/Request/StorageRequest.cpp HDFS/Request/StorageRequest.h HDFS/Common/Utils.cpp HDFS/Common/Utils.h HDFS/Request/Request.cpp HDFS/Connection/ConnectionManager.cpp HDFS/Connection/ConnectionManager.h HDFS/Connection/Connection.cpp HDFS/Connection/Connection.h HDFS/Response/Response.cpp HDFS/Response/Response.h HDFS/Data/DataStore.cpp HDFS/Data/DataStore.h HDFS/Data/DataValue.cpp HDFS/Data/DataValue.h HDFS/Request/RetrievalRequest.cpp HDFS/Request/RetrievalRequest.h HDFS/HDFSServiceImpl.cpp HDFS/HDFSServiceImpl.h Utils/Utils.h Utils/Utils.cpp Common/EnumAndConstants.cpp)
target_link_libraries(HDFS PUBLIC ${Boost_LIBRARIES} Threads::Threads grpc++ mr_grpc_proto ${TARGET_LOGGER})
add_executable(Master Master/Master.cpp Master/MapReduceService/MapReduceImpl.cpp Master/main.cpp Map/Config/MapConfig.cpp Map/Config/MapConfig.h Common/File/File.cpp Common/File/File.h Master/Models/Mapper.cpp Master/Models/Mapper.h Common/EnumsAndConstants.h Common/ServiceAddress.h Master/StatusChecker/StatusChecker.cpp Master/StatusChecker/StatusChecker.h HDFS/HDFSServiceImpl.cpp HDFS/HDFSServiceImpl.h Map/MapperServiceImpl.cpp Map/MapperServiceImpl.h Master/MapReduceService/MapReduceImpl.h Utils/Utils.h Utils/Utils.cpp Master/Models/Reducer.cpp Master/Models/Reducer.h Reduce/Config/ReduceConfig.h Reduce/Config/ReduceConfig.cpp Common/EnumAndConstants.cpp)
target_link_libraries(Master PUBLIC ${Boost_LIBRARIES} Threads::Threads grpc++ mr_grpc_proto ${TARGET_LOGGER})
add_executable(Mapper Master/Master.cpp Map/Config/MapConfig.cpp Map/Config/MapConfig.h Common/File/File.cpp Common/File/File.h Master/Models/Mapper.cpp Master/Models/Mapper.h Common/EnumsAndConstants.h Common/ServiceAddress.h Master/StatusChecker/StatusChecker.cpp Master/StatusChecker/StatusChecker.h Map/main.cpp HDFS/HDFSServiceImpl.cpp HDFS/HDFSServiceImpl.h Map/MapperServiceImpl.cpp Map/MapperServiceImpl.h Map/Core/Map.cpp Map/Core/Map.h Map/Core/UserProvidedMapFunction.cpp Utils/Utils.h Utils/Utils.cpp Master/Models/Reducer.cpp Master/Models/Reducer.h Reduce/Config/ReduceConfig.h Reduce/Config/ReduceConfig.cpp Common/EnumAndConstants.cpp)
target_link_libraries(Mapper PUBLIC ${Boost_LIBRARIES} Threads::Threads grpc++ mr_grpc_proto ${TARGET_LOGGER})
add_executable(Reducer Reduce/Config/ReduceConfig.cpp Reduce/Config/ReduceConfig.h Reduce/Core/Reduce.cpp Reduce/Core/Reduce.h Reduce/Core/UserProvidedReduceFunction.cpp Reduce/ReducerServiceImpl.cpp Reduce/ReducerServiceImpl.h Reduce/main.cpp Utils/Utils.cpp Utils/Utils.h Common/EnumAndConstants.cpp)
target_link_libraries(Reducer PUBLIC ${Boost_LIBRARIES} Threads::Threads grpc++ mr_grpc_proto ${TARGET_LOGGER})

add_custom_target(All)
add_dependencies(All HDFS Master Mapper Reducer)

#add_executable(Reducer)
#target_include_directories(Assignment2 PUBLIC ${Boost_INCLUDE_DIR})
#target_link_libraries(Assignment2 ${Boost_LIBRARIES} Threads::Threads)
#