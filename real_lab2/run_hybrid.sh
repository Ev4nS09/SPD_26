#!/bin/bash

dir="times/Hybrid"
mkdir -p "$dir"

for (( m_size=$1; m_size<=$2; m_size+=$3 )); do
    echo "Doing game of Life of matrix size ${m_size}x${m_size}"

    tmp_file=$dir/tmp_file.tmp
    time_file=$dir/GoL_MPI_${m_size}x${m_size}.time
    : > $tmp_file
    : > $time_file

    for (( gen=0; gen<=$4; gen++ )); do
        log_file=$dir/tmp_run_${m_size}_${gen}.log
        
        { time mpirun  --host localhost:1,rio@192.168.1.132:1 -np 2 --map-by ppr:1:node:PE=4 --bind-to core -wd /MPI ./Life --no-display -r $m_size -c $m_size -g 128 -i example_spaceships.in ; } 2> $log_file
        
        # Extract 'real' time from the log file and append to tmp_file
        grep "real" $log_file | sed 's/,/./' >> $tmp_file
    	rm $log_file
    done

    ./bin/make_time_file $tmp_file $time_file $4
    rm $tmp_file
done
