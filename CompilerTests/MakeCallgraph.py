#!/usr/bin/python

from subprocess import run, TimeoutExpired
from os.path import isfile

from sys import argv

defaultArgs = ["../Compiler/build/Debug/DafCompiler", "TestFile.daf"]
timeout = 2
tmpFile = "callgrind.out.tmp"

if len(argv) > 2: #script binary args
    args = argv[1:]
else:
    args = defaultArgs
    if len(argv) > 1:
        args[0] = argv[1] #Overrite binary only

def doStuff():
    try:
        run(["valgrind", "--tool=callgrind", "--callgrind-out-file="+tmpFile]+args, timeout=timeout)
    except TimeoutExpired:
        print("Valgrind callgrind timed out after", timeout, "seconds")
        return 1
    if not isfile(tmpFile):
        print("Failed to run callgrind")
        return 1
    run(["kcachegrind", tmpFile])
    return 0

code = doStuff()
if isfile(tmpFile):
    run(["rm", tmpFile])

exit(code)
