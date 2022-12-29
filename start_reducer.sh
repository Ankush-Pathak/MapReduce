#!/bin/bash
# GRPC_TRACE=all
# GRPC_VERBOSITY=DEBUG
cd .mapreduce
nohup ./Reducer $1 >> "reducer$1.log" 2>&1 &
