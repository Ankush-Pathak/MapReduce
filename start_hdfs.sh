#!/bin/bash
# export GRPC_TRACE=all
# export GRPC_VERBOSITY=debug
cd .mapreduce
nohup ./HDFS "$1" clear >> "hdfs$1.log" 2>&1 &
