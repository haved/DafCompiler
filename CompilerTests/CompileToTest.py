#!/usr/bin/python

#arguments specified like this:
#./CompileToTest.py [build folder] [cmake source dir from build] [cmake options]

##Last modified 2016-11-11
##No, 2016-12-03
###Noo, 2016-12-04

exec_name = "CompileToTest.py"

cmakeTarget = "Unix Makefiles" #Ninja
makeCommand = ["make", "-j3"] #["ninja"]
linker = ""
export_compile_commands = True

from subprocess import call
from sys import argv
from os.path import isdir
from os import makedirs, chdir

buildDir = "TestBuild"
cmakeRelative = "../../Compiler/"

opt = argv[1:]

if len(opt) > 0:
    if len(opt) < 2:
        print("Error! Need to specify <build folder> <cmake dir relative to build folder> [other cmake settings]")
        exit(1)
    buildDir = opt[0]
    cmakeRelative = opt[1]

if not isdir(buildDir):
    makedirs(buildDir)
chdir(buildDir)

hasLLD = call(["ld.lld", "--version"]) == 0
if hasLLD:
    linker="ld.lld"

cmakeCallList = ["cmake", cmakeRelative]
if len(linker) > 0:
    cmakeCallList += ["-DCMAKE_LINKER="+linker]
if len(opt)>2:
    cmakeCallList+=opt[2:]
cmakeCallList+=["-G"+cmakeTarget]
if  export_compile_commands:
    cmakeCallList+=["-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"]

print(exec_name, "cmake command: ", " ".join(cmakeCallList))

if call(cmakeCallList) != 0:
    print(exec_name, "fatal error in cmake")
    exit(1)

print(exec_name, "make command: ", " ".join(cmakeCallList))

if call(makeCommand) != 0:
    print(exec_name, "fatal error in make")
    exit(1)
