#!/bin/bash

rm bin/make_time_file

gcc -Wall -O3 make_time_file.c -o make_time_file -lm 

mv make_time_file bin/
