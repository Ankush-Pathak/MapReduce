#!/bin/bash

wget https://www.gutenberg.org/cache/epub/145/pg145.txt -O seed_file
sed -i "s/[^a-zA-Z0-9\s ]//g" seed_file


truncate input_file -s 0
while [ `stat -c '%s' input_file` -le $1 ]
do
    cat seed_file |  shuf >> input_file
done
