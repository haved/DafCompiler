#!/usr/bin/bash 
if ./CompileToTest.py; then
./RunTests.py -A -f ".*\.daf(\.test)?$"
fi
exit $?
