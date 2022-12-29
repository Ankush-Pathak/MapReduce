#!/bin/bash
mkdir -p .local && mkdir -p .local/bin
PATH="$PWD/.local/bin:$PATH"
guards="------------------------------------------"
pre_guards="\n\n$guards\n"
post_guards="\n$guards\n\n"
MASTER_PID=""
MASTER_PORT="${MASTER_PORT:-4100}"
# GRPC_TRACE=all
# GRPC_VERBOSITY=DEBUG

function setupTools() {
  printf $pre_guards
  echo "Setting up grpc-client-cli"
  curl -L https://github.com/vadimi/grpc-client-cli/releases/download/v1.15.0/grpc-client-cli_linux_x86_64.tar.gz | tar -C .local/bin -xz
  EXIT_CODE=$?
  printf $post_guards
  return $EXIT_CODE
}

function build() {
  printf $pre_guards
  echo "Building code..."
  mkdir -p ../build && cd ../build
  cmake -j 15 -DCMAKE_BUILD_TYPE=Release -B. -S.. && make -j 15 && echo "Build done" 
  cd -
  EXIT_CODE=$?
  printf $post_guards
  return $EXIT_CODE
}

function generateInput() {
  printf $pre_guards
  echo "Generating input"
  TENMB=10485760 
  TWENTYMB=20971520
  echo "Generating 10MB file"
  bash input_generator.sh $TENMB
  mv input_file ../build/input_file_10MB
  echo "Generating 20MB file"
  bash input_generator.sh $TWENTYMB
  mv input_file ../build/input_file_20MB
  printf $post_guards

}

function killProcesses() {
  printf $pre_guards
  echo "Cleaning up residual processes, this may cause armageddon"
  USER=$(whoami)
  set -x
  ps -f -u $USER  | grep "Mapper\|Master\|HDFS\|Reducer" | grep -v grep | awk '{print $2}' | xargs -t kill -9
  set +x
  printf $post_guards
}

function startMaster() {
  printf $pre_guards
  echo "Starting master"
  cd ../build
  timeout 300  ./Master $MASTER_PORT 2>&1 >master$MASTER_PORT.log &
  cd -
  MASTER_PID=$!
  printf $post_guards
}

function resetEnv() {
  printf $pre_guards
  echo "Resetting environment"
  TS=$(date +%Y%m%d_%H_%M_%S_%3N)
  set -x
  mv ~/.mapreduce ~/.mapreduce$TS
  mkdir ~/.mapreduce$TS/home_logs
  rm -rf "~/.mapreduce/cache.store*"
  for f in ~/hdfs*log; do
      mv "$f" ~/.mapreduce$TS/home_logs
  done
  for f in ~/reducer*log; do
      mv "$f" ~/.mapreduce$TS/home_logs
  done
  for f in ~/mapper*log; do
      mv "$f" ~/.mapreduce$TS/home_logs
  done
  mkdir ~/.mapreduce
  mkdir ../build/backup$TS
  if [[ -f ../build/output ]]; then
      mv ../build/output ../build/backup$TS
  fi
  for f in ../build/*.log; do
    mv -t ../build/backup$TS "$f" 
  done
  for f in ../build/cache.store*; do
    mv -t ../build/backup$TS "$f" 
  done
  set +x
  printf $post_guards
}

function createConfig() {
  printf $pre_guards
  echo "Creating Map Reduce config"
  cp start_req.json.template start_req.json
  INPUT_FILE=$1
  MAP_FILE=$2
  RED_FILE=$3
  sed -i "s|_input_file_|$INPUT_FILE|g" start_req.json
  sed -i "s|_map_file_|$MAP_FILE|g" start_req.json
  sed -i "s|_red_file_|$RED_FILE|g" start_req.json
  printf $post_guards
}

function startMapReduce() {
  printf $pre_guards
  echo "Making gRPC request to start MapReduce job"
  grpc-client-cli --proto ../Common/Services/MapReduce.proto -i start_req.json -method startMapReduce -service MapReduce localhost:$MASTER_PORT
  printf $post_guards
}

function monitorMaster() {
  printf $pre_guards
  echo "Starting Master monitor"
  MASTER_LOG="../build/master$MASTER_PORT.log"
  while true
  do
    if ps -p $MASTER_PID > /dev/null
    then
      set -x 
      tail $MASTER_LOG 
      set +x
      if grep -q "Run took" $MASTER_LOG;
      then
        echo "Job done"
        kill -9 $MASTER_PID
        break
      fi
    else
      echo "Master exited"
      break
    fi
    sleep 11
  done
  printf $post_guards
}

function main() {
  build
  setupTools
  generateInput

  APP=("Word Count" "Inverted Index")
  REDUCE_FILES=("$PWD/wordcountRed.cpp" "$PWD/invertedindexRed.cpp")
  MAP_FILES=("$PWD/wordcountMap.cpp" "$PWD/invertedindexMap.cpp")
  INPUT_FILES=($(ls $PWD/../build/input_file*))
  for index in {0..1}
  do
    for input in "${INPUT_FILES[@]}"
    do
      echo "Starting ${APP[$index]} with input $input"
      sleep 3
      createConfig "$input" "${MAP_FILES[$index]}" "${REDUCE_FILES[$index]}"
      startMaster
      startMapReduce
      monitorMaster
      sleep 3
      killProcesses
      sleep 10
      resetEnv
      sleep 2
      echo "${APP[$index]} with input $input done"
      sleep 3
    done
  done
}





main
