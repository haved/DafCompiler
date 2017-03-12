#!/usr/bin/bash 
if ./CompileToTest.py; then
./RunTests.py -A -M -f ".*\.daf(\.test)?$"
fi
exit $?
