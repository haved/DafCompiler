#!/usr/bin/bash 
./CompileToTest.py
if $?; then
	./RunTests.py -A -M -f ".*\.daf(\.test)?$"
fi
exit $?
