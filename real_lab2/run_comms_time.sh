#!/bin/bash

dir="times/comms"
mkdir -p "$dir"

tmp_file="$dir/tmp_file.tmp"
time_file="$dir/Communication.time"

: > "$tmp_file"
: > "$time_file"

for (( trial=0; trial<=$1; trial++ ))
do
    log_file="$dir/tmp_run.log"

    { time mpiexec -v --host localhost:1,rio@192.168.1.132:1 -np 2 -wd /MPI ./coms_time ; } 2> "$log_file"

    grep "real" "$log_file" | sed 's/,/./' >> "$tmp_file"
    rm "$log_file"

done

./bin/make_time_file "$tmp_file" "$time_file" $1

rm $tmp_file
