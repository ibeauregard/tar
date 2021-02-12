#!/bin/bash

if [ $# -eq 0 ]; then
	printf "ERROR: Include at least file or directory as an argument\n"
	exit 1 
fi

# Delete intermediate files before script ends
cleanUp() {
	for arg in "$@"; do
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

# TEST 0: Compiling my_tar
testMake() {
	cd ..
	make re --silent
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 0: Compiling my_tar\n"
		echo $res
		exit 1
	fi
	printf "PASSED TEST 0: Compiling my_tar\n"
	cd - >> /dev/null 2>&1
}

# TEST 1: Creating tar archive
testCMode() {
	for arg in "$@"
	do
		tar -f $arg.tar -c $arg
		hexdump -C $arg.tar > $arg.txt
		../my_tar -f my_$arg.tar -c $arg
		hexdump -C my_$arg.tar > my_$arg.txt
		res=$(cmp -b <(head -n -1 $arg.txt) <(head -n -1 my_$arg.txt))
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 1: Creating tar archive\n"
			echo $res
			exit 1
		fi
	done
	printf "PASSED TEST 1: Creating tar archive\n"
	cleanUp "$@"
}

# TEST 2: Append tar contents
testRMode() {
	for arg in "$@"
	do
		# New Archive
		tar -rf new.tar $arg
		hexdump -C new.tar > new.txt
		../my_tar -rf my_new.tar $arg
		hexdump -C my_new.tar > my_new.txt
		res=$(cmp -b <(head -n -1 new.txt) <(head -n -1 my_new.txt))
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 2: Append tar (new archive)\n"
			echo $res
			exit 1
		fi
		rm new.*
		rm my_new.*

		# Existing Archive
		tar -f $arg.tar -c $arg
		tar -rf $arg.tar emptytest filetest dirtest
		hexdump -C $arg.tar > $arg.txt
		../my_tar -cf my_$arg.tar $arg
		../my_tar -rf my_$arg.tar emptytest filetest dirtest
		hexdump -C my_$arg.tar > my_$arg.txt
		res=$(cmp -b <(head -n -1 $arg.txt) <(head -n -1 my_$arg.txt))
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 2: Append tar (existing archive)\n"
			echo $res
			exit 1
		fi
	done
	printf "PASSED TEST 2: Append tar contents\n"
	cleanUp "$@"
}

# TEST 3: Update tar contents
testUMode() {
	for arg in "$@"
	do
		# New Archive
		tar -uf new.tar $arg
		hexdump -C new.tar > new.txt
		../my_tar -uf my_new.tar $arg
		hexdump -C my_new.tar > my_new.txt
		res=$(cmp -b <(head -n -1 new.txt) <(head -n -1 my_new.txt))
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 3: Update tar (empty archive)\n"
			echo $res
			cleanUp "$@"
			exit 1
		fi
		rm new.*
		rm my_new.*

		# Existing Archive
		tar -f $arg.tar -c $arg
		../my_tar -f my_$arg.tar -c $arg
		## Check pre-update
		echo "File contents might update" > uFlag/yesUpdate
		echo "File contents will not update" > uFlag/noUpdate
		tar -uf $arg.tar uFlag/empty uFlag/yesUpdate uFlag/noUpdate 
		../my_tar -uf my_$arg.tar uFlag/empty uFlag/yesUpdate uFlag/noUpdate 
		hexdump -C $arg.tar > $arg.txt
		hexdump -C my_$arg.tar > my_$arg.txt
		res=$(cmp -b <(head -n -1 $arg.txt) <(head -n -1 my_$arg.txt))
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 3: Update tar (pre-update)\n"
			echo $res
			exit 1
		fi
		## Check post-update
		echo "File contents have been updated" > uFlag/yesUpdate
		tar -uf $arg.tar uFlag/empty uFlag/yesUpdate uFlag/noUpdate 
		../my_tar -uf my_$arg.tar uFlag/empty uFlag/yesUpdate uFlag/noUpdate 
		hexdump -C $arg.tar > $arg.txt
		hexdump -C my_$arg.tar > my_$arg.txt
		res=$(cmp -b <(head -n -1 $arg.txt) <(head -n -1 my_$arg.txt))
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 3: Update tar (post-update)\n"
			echo $res
			exit 1
		fi
	done
	printf "PASSED TEST 3: Update tar contents\n"
	cleanUp "$@"
}

# TEST 4: Listing tar contents 
testTMode() {
	for arg in "$@"
	do
		tar -tf $arg.tar > $arg.txt
		../my_tar -tf $arg.tar > my_$arg.txt
		res=$(cmp -b $arg.txt my_$arg.txt) 
		status=$?
		if [ $status -gt 0 ]; then
			printf "FAILED TEST 4: Listing tar contents\n"
			echo $res
			exit 1
		fi
	done
	printf "PASSED TEST 4: Listing tar contents\n"
}

# TEST 5: Extract tar contents
testXMode() {
	for arg in "$@"
	do
		tar -f $arg.tar -c $arg
		../my_tar -f my_$arg.tar -c $arg

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
			printf "FAILED TEST 5: Extract tar (missing files)\n"
			echo $res
			exit 1
		fi

		# Ensure contents of files are the same
		while read pathname;
		do 
			res=$(cmp -b extract_tar/$pathname extract_my_tar/$pathname) 
			status=$?
			if [ $status -gt 0 ]; then
				printf "FAILED TEST 5: Extract tar (bad contents)\n"
				echo $res
				exit 1
			fi
		done < extract_tar_contents.txt
	done
	printf "PASSED TEST 5: Extract tar contents\n"
}

cmpStdErr() {
	for comm in "$@"
	do
	# cut: errors start with either "tar: " or "my_tar: " that we exclude
		echo ${comm}: >> errors.txt
		echo ${comm}: >> my_errors.txt
		$comm 2>&1 >/dev/null | cut -d' ' -f2- >> errors.txt
		# sed: tar errors includes a second line "try --help" that we exclude 
		helpMessage=$(tail -n -1 errors.txt | cut -d' ' -f1-2)
		if [[ $helpMessage == "'tar --help'" ]]; then
			sed -i '$d' errors.txt
		fi
		../my_$comm 2>&1 >/dev/null | cut -d' ' -f2- >> my_errors.txt
	done
}

# TEST 6: Error messages
testErrors() {
	rm errors >> /dev/null 2>&1
	rm my_errors >> /dev/null 2>&1
	touch errors.txt
	touch my_errors.txt
	cmpStdErr "tar -f" "tar -c foo.txt -f" "tar -c -f" "tar -cf"
	cmpStdErr "tar" "tar foo.txt" "tar -y" "tar -yc" "tar -cy"
	cmpStdErr "tar -ct" "tar -c" "tar -cf foo.tar foo.inexistent"
	cmpStdErr "tar -rf foo.tar foo.inexistent" "tar -uf foo.tar foo.inexistent"
	cmpStdErr "tar -cf foo.tar foo.tar" "tar -rf foo.tar foo.tar"
	cmpStdErr "tar -uf foo.tar foo.tar"
	cmpStdErr "tar -r" "tar -r foo.txt" "tar -u" "tar -u foo.txt"
	diff -1 errors.txt my_errors.txt
	status=$?
	if [ $status -gt 0 ]; then
		printf "FAILED TEST 6: Error Messages\n"
		exit 1
	fi
	printf "PASSED TEST 6: Error Messages\n"
}

testMake
testCMode "$@"
testRMode "$@"
testUMode "$@"
# testTMode "$@"
testXMode "$@"
# testErrors
cleanUp "$@"
exit 0 
