#!/bin/bash

export OMP_NUM_THREADS=$5

dir="times/OpenMP/$5_threads"

if [[ ! -d $dir ]]
then
    mkdir $dir
fi

for (( m_size=$1; m_size<=$2; m_size+=$3 ))
do


    if [[ ! -d $dir ]]
    then
        mkdir $dir
    fi

    tmp_file=$dir/tmp_file.tmp
	time_file=$dir/GoL_OpenMP_$5_threads_${m_size}x${m_size}.time

    echo '' > $tmp_file
    echo '' > $time_file

	echo "Doing game of Life of matrix size ${m_size}x${m_size} with $5 threads"

    for (( gen=0; gen<=$4; gen+=1 ))
    do
        (time ./GoL_OpenMP/Life --no-display -r $m_size -c $m_size -g 128) |& grep "real" | sed 's/,/./' >> $tmp_file
    done

    ./bin/make_time_file $tmp_file $time_file

    rm $tmp_file

done
