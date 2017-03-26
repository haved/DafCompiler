#!/usr/bin/python

#arguments specified like this:
#./CompileToTest.py [build folder] [cmake source dir from build] [cmake options]

##Last modified 2016-11-11
##No, 2016-12-03
###Noo, 2016-12-04

cmakeTarget = "Unix Makefiles" #Ninja
makeCommand = ["make", "-j3"] #["ninja"]

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

if call(["cmake", cmakeRelative]+(opt[2:]if len(opt)>2 else [])+["-G"+cmakeTarget, "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"]) != 0:
    exit(1)
if call(makeCommand) != 0:
    exit(1)
