#!/usr/bin/python

from os import listdir
from os.path import isfile
from sys import argv

from subprocess import run, TimeoutExpired

files = listdir(".")

if len(argv)>2:
    print("Too many arguments passed")
execu = argv[1] if len(argv)>=2 else "../Compiler/build/Debug/DafCompiler"

for inx, file_ in enumerate([file_ for file_ in files if isfile(file_) and file_[-4:]==".daf"]):
    try:
        code = run([execu, file_], timeout=2).returncode
        if code is not 0:
            print("ERROR: Test (",inx+1,"/",len(file_),") returned code ",code,": ",file_, sep='')
            exit()
    except TimeoutExpired:
        print("ERROR: Test (",inx+1,"/",len(file_),") timed out: ", file_, sep='')
        exit()

print("All tests successful")
