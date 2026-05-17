#!/bin/bash

dir="times/Sequencial"

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
        (time ./GoL_Serial/Life --no-display -r $m_size -c $m_size -g 512 -i $5) 2>&1 | (head -n1; grep "real") | sed 's/,/./' >> $tmp_file
    done

    ./bin/make_time_file $tmp_file $time_file $4

    if [[ $? == 1 ]]
    then
        rm $time_file
        m_size=$m_size-$3
        echo "Error: standard deviation too high, redoing game of life..."
    fi

    rm $tmp_file
done
