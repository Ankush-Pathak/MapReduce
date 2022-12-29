#!/bin/bash

ORIG_MAP_FILE="UserProvidedMapFunction.cpp"
ORIG_RED_FILE="UserProvidedReduceFunction.cpp"
MAP_PATH="Map/Core/"
RED_PATH="Reduce/Core/"
# GRPC_TRACE=all
# GRPC_VERBOSITY=DEBUG
cd ..
if [[ ! -f "$MAP_PATH$ORIG_MAP_FILE.orig" ]]; then
  mv $MAP_PATH$ORIG_MAP_FILE $MAP_PATH$ORIG_MAP_FILE.orig
fi

if [[ ! -f "$RED_PATH$ORIG_RED_FILE.orig" ]]; then
  mv $RED_PATH$ORIG_RED_FILE $RED_PATH$ORIG_RED_FILE.orig
fi

cp $1 $MAP_PATH$ORIG_MAP_FILE
cp $2 $RED_PATH$ORIG_RED_FILE
cd build 
make Mapper Reducer -j 15
cd -
mv $RED_PATH$ORIG_RED_FILE.orig $RED_PATH$ORIG_RED_FILE
mv $MAP_PATH$ORIG_MAP_FILE.orig $MAP_PATH$ORIG_MAP_FILE

