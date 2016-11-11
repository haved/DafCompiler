#!/usr/bin/bash 
./CompileToTest.py
./RunTests.py -A -M -f ".*\.daf(\.test)?"
exit $?
