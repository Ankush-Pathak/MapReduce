#!/bin/bash
# export GRPC_TRACE=all
# export GRPC_VERBOSITY=debug
cd .mapreduce
nohup ./Mapper $1 >> "mapper$1.log" 2>&1 &
