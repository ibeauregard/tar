#!/bin/bash

if [ $# -eq 0 ]; then
	printf "ERROR: Include at least file or directory as an argument\n"
	exit 1 
fi

# TEST 1: Creating tar archive
for arg in $@
do
	name=arg
	tar -f $arg.tar -c $arg
	# TODO: Uncomment next line when my_tar binary is available
	# ../my_tar -f my_$arg.tar -c $arg
	# TODO: Change 2nd arg of next line when my_tar becomes available
	res=$(cmp -b $arg.tar dirtest.tar) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 1: Creating tar archive\n"
		echo $res
		exit 1
	fi
done
printf "PASSED TEST 1: Creating tar archive\n"

# TEST 2: Listing tar contents 
for arg in $@
do
	name=arg
	tar -tf $arg.tar > $arg.txt
	# TODO: Uncomment next line when my_tar binary is available
	# ../my_tar -tf $arg.tar > my_$arg.txt
	# TODO: Change 2nd arg of next line when my_tar becomes available
	res=$(cmp -b $arg.txt filetest.txt) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 2: Listing tar contents\n"
		echo $res
		exit 1
	fi
done
printf "PASSED TEST 2: Listing tar contents\n"

# Return total number of errors
exit 0 
