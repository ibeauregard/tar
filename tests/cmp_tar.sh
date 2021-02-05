#!/bin/bash

if [ $# -eq 0 ]; then
	printf "ERROR: Include at least file or directory as an argument\n"
	exit 1 
fi

# Delete intermediate files before script ends
cleanUp() {
	for arg in ${BASH_ARGV[*]}
	do
        rm $arg.txt >> /dev/null 2>&1
		rm my_$arg.txt >> /dev/null 2>&1
		rm $arg.tar >> /dev/null 2>&1
		rm my_$arg.tar >> /dev/null 2>&1
	done
	rm extract_tar_contents.txt >> /dev/null 2>&1
	rm extract_my_tar_contents.txt >> /dev/null 2>&1
	rm -r extract_tar >> /dev/null 2>&1
	rm -r extract_my_tar >> /dev/null 2>&1
}

# TEST 1: Creating tar archive
for arg in $@
do
	name=arg
	tar -f $arg.tar -c $arg
	hexdump -C $arg.tar > $arg.txt
	../my_tar -f my_$arg.tar -c $arg
	hexdump -C my_$arg.tar > my_$arg.tar
	res=$(cmp -b $arg.txt my_$arg.txt) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 1: Creating tar archive\n"
		echo $res
		cleanUp
		exit 1
	fi
done
printf "PASSED TEST 1: Creating tar archive\n"

# TEST 2: Listing tar contents 
for arg in $@
do
	name=arg
	tar -tf $arg.tar > $arg.txt
	../my_tar -tf $arg.tar > my_$arg.txt
	res=$(cmp -b $arg.txt my_$arg.txt) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 2: Listing tar contents\n"
		echo $res
		cleanUp
		exit 1
	fi
done
printf "PASSED TEST 2: Listing tar contents\n"

# TEST 3: Append tar contents
for arg in $@
do
	name=arg
	tar -f $arg.tar -c $arg
	tar -rf $arg.tar emptytest filetest dirtest
	../my_tar -cf my_$arg.tar $arg
	../my_tar -rf my_$arg.tar emptytest filetest dirtest
	res=$(cmp -b $arg.tar my_$arg.tar) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 3: Append tar contents\n"
		echo $res
		cleanUp
		exit 1
	fi
done
printf "PASSED TEST 3: Append tar contents\n"

# TEST 4: Update tar contents
for arg in $@
do
	name=arg
	tar -uf $arg.tar u_flag/emptytest u_flag/filetest
	../my_tar -uf $arg.tar u_flag/emptytest u_flag/filetest
	res=$(cmp -b $arg.tar my_$arg.tar) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 4: Update tar contents\n"
		echo $res
		cleanUp
		exit 1
	fi
done
printf "PASSED TEST 4: Update tar contents\n"

# TEST 5: Extract tar contents
for arg in $@
do
	name=arg
	# Extract contents using tar
	mkdir extract_tar >> /dev/null 2>&1
	cd extract_tar
	tar -xf ../$arg.tar 
	cd ..

	# Extract contents using my_tar
	mkdir extract_my_tar >> /dev/null 2>&1
	cd extract_my_tar
	../../my_tar -xf ../my_$arg.tar 
	cd ..

	# Ensure contents of directories are the same
	cd extract_tar
	find * -type f -exec echo "{}" \; > ../extract_tar_contents.txt
	cd ../extract_my_tar
	find * -type f -exec echo "{}" \; > ../extract_my_tar_contents.txt
	cd ..
	res=$(cmp -b extract_tar_contents.txt extract_my_tar_contents.txt) 
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 5: Extract tar contents (missing files)\n"
		echo $res
		cleanUp
		exit 1
	fi

	# Ensure contents of files are the same
	while read pathname;
	do 
		res=$(cmp -b extract_tar/$pathname extract_my_tar/$pathname) 
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 5: Extract tar contents (bad contents)\n"
			echo $res
			cleanUp
			exit 1
		fi
	done < extract_tar_contents.txt
done
printf "PASSED TEST 5: Extract tar contents\n"

cleanUp
exit 0 
