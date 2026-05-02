#!/bin/bash


dir="times/MPI"

if [[ ! -d $dir ]]
then
    mkdir $dir
fi

for (( m_size=$1; m_size<=$2; m_size+=$3 ))
do
	echo "Doing game of Life of matrix size ${m_size}x${m_size}"


    if [[ ! -d $dir ]]
    then
        mkdir $dir
    fi

    tmp_file=$dir/tmp_file.tmp
	time_file=$dir/GoL_Serial_${m_size}x${m_size}.time

    echo '' > $tmp_file
    echo '' > $time_file

    for (( trial=0; trial<=$4; trial+=1 ))
    do
        (time mpiexec -v --host fct-deei-linux:1 --host fct-deei-aval:1 -np 2 -wd /home/a76943/SPD_26/GoL_MPI ./GoL_MPI/Life --no-display -r $m_size -c $m_size -g 128)  |& grep "real" | sed 's/,/./' >> $tmp_file
    done

    ./bin/make_time_file $tmp_file $time_file $4

    rm $tmp_file

done
