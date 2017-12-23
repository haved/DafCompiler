#!/usr/bin/python

from sys import argv, stdout, stderr
from subprocess import Popen, run, TimeoutExpired, PIPE
from os import remove

release = False
binary = "TestBuild/DafCompiler"

opt = argv[1:]

if len(opt) >= 1:
    if(opt[0]=="-r"):
        release = True
        binary = "ReleaseBuild/DafCompiler"
        opt=opt[1:]
        print("Release run")


def flush():
    stdout.flush()
    stderr.flush()

if len(opt) != 1:
    print("Usage: RunTest.py [-r] <daf file>")
    exit(1)

inFile = opt[0]
outputObjectFile = "compiledDaf.o"
dafMainCallerSrc = "dafMainCaller.cpp"
dafMainCallerObj = "dafMainCaller.o"
execOut = "dafExec.out"

printProgramOutput = True;

try:
    code = run([binary, inFile, "-o", outputObjectFile], timeout=2, stdout=stdout if printProgramOutput else PIPE, stderr=stderr if printProgramOutput else PIPE).returncode
    flush();
    if code is not 0:
        print("ERROR: daf compilation returned with error code", code);
        exit(1);
except TimeoutExpired:
    print("ERROR: daf compilation timed out")
    flush()
    exit(1)

try:
    print("TEST: compiling C++")
    code = run(["g++", dafMainCallerSrc, "-o", dafMainCallerObj, "-c"], timeout=5, stdout=stdout if printProgramOutput else PIPE).returncode
    flush();
    if code is not 0:
        print("ERROR: Compiling C++ part failed", code)
        exit(1);
except TimeoutExpired:
    print("ERROR: Compiling the c++ part timed out");
    flush();
    exit(1);


try:
    code = run(["g++", outputObjectFile, dafMainCallerObj, "-o", execOut], timeout=5, stdout=stdout if printProgramOutput else PIPE).returncode
    flush();
    if code is not 0:
        print("ERROR: Linking failed with error code", code)
        exit(1);
except TimeoutExpired:
    print("ERROR: Linking with g++ timed out");
    flush();
    exit(1);

try:
    code = run(["./"+execOut]).returncode
    print("RETURNED with return code", code)
except TimeoutExpired:
    print("ERROR: program execution timed out");
    flush();
    exit(1);

remove(outputObjectFile)
print("Removed", outputObjectFile)
remove(dafMainCallerObj)
print("Removed", dafMainCallerObj);
