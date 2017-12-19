#!/usr/bin/python

from os import listdir
from os.path import isfile, isdir
from sys import argv, stdout, stderr

from subprocess import Popen, run, TimeoutExpired, PIPE

from re import compile

binary = "TestBuild/DafCompiler" if isdir("TestBuild/") else "../Compiler/build/Debug/DafCompiler"
filterText =  ".*"
allowOtherThanDaf = False
memcheck = False
printProgramOutput = True
scriptOutput = stdout

args = argv[1:]

def flush():
    stdout.flush()
    stderr.flush()

def printHelpMessage():
    print("Usage: ./RunTests.py [options]")
    print("Options:")
    print("-h --help         Print this help message and exit")
    print("-x <exutable>     Set the executable")
    print("-f <regex filter> Filter what files are included")
    print("-A                Allow files without .daf ending")
    print("-M                Do valgrind memcheck, only printed if problem (to stderr)")
    print("-s                Don't print program stdout")
    print("-e                Use stderr to print output of this script.")
    exit(0)

def getNextArg(args, i):
    assert(i<len(args))
    if i+1 == len(args):
       print("Expected a parameter after", args[i])
       exit(1)
    else:
        return args[i+1]

i=0
while i < len(args):
    arg = args[i]
    if arg == "--help" or arg == "-h":
        printHelpMessage()
    elif arg == "-x":
        executable = getNextArg(args, i)
        i+=1
    elif arg == "-f":
        filterText = getNextArg(args, i)
        i+=1
    elif arg == "-A":
        allowOtherThanDaf = True
    elif arg == "-M":
        memcheck = True
    elif arg == "-s":
        printProgramOutput = False
    elif arg == "-e":
        scriptOutput = stderr
    else:
        print("Parameter not reconized:", arg)
    i+=1

filter = compile(filterText)
files = [file_ for file_ in listdir(".") if isfile(file_) and (allowOtherThanDaf or file_[-4:]==".daf") and filter.match(file_)]

flush()

if not memcheck:
    for inx, file_ in enumerate(files):
        print("Running Test (",inx+1,"/",len(files),": ", file_,")", sep='',file=scriptOutput)
        flush()
        try:
            code = run([binary, file_], timeout=2, stdout=stdout if printProgramOutput else PIPE).returncode
            flush()
            if code is not 0:
                print("ERROR: Test (",inx+1,"/",len(files),": ",file_,") returned code: ", code, sep='',file=scriptOutput)
                flush()
                exit(1)
        except TimeoutExpired:
            print("ERROR: Test (",inx+1,"/",len(files),": ",file_,") timed out", sep='',file=scriptOutput)
            flush()
            exit(1)
else: #memcheck!
    for inx, file_ in enumerate(files):
        print("Running memcheck (",inx+1,"/",len(files),": ",file_,")",sep='',file=scriptOutput)
        flush()
        try:
            program = run(["valgrind", "--leak-check=full", "--show-leak-kinds=all", binary, file_], stdout=stdout if printProgramOutput else PIPE, stderr=PIPE, universal_newlines=True, timeout=15)
            flush()
            if program.returncode != 0: #The compiler faulted, don't print valgrind
                print("ERROR: Test (",inx+1,"/",len(files),": ",file_,") returned code: ", program.returncode, sep='',file=scriptOutput)
                flush()
                exit(1)
            flush()
            if "no leaks are possible" not in program.stderr:
                print(program.stderr, file=stderr)
                print("ERROR: Memory leak found in (",inx+1,"/",len(files),": ", file_, ")",sep='')
                flush()
                exit(1)
        except TimeoutExpired:
            print("ERROR: Test (",inx+1,"/",len(files),": ",file_,") timed out", sep='',file=scriptOutput)
            flush()
            exit(1)

print("All tests (",len(files),") successful", sep='',file=scriptOutput)
flush()
