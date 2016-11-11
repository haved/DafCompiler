#!/usr/bin/python

##Last modified 2016-11-11

from subprocess import call
from sys import argv
from os.path import isdir
from os import makedirs, chdir

buildDir = "../Compiler/build/Testing"
cmakeRelative = "../.."

if len(argv) > 1:
    if len(argv) < 3:
        print("Error! Need to specify <build folder> <cmake dir relative to build folder> [other cmake settings]")
        exit(1)
    buildDir = argv[1]
    cmakeRelative = argv[2]

if not isdir(buildDir):
    makedirs(buildDir)
chdir(buildDir)

call(["cmake", cmakeRelative]+(argv[3:]if len(argv)>3 else [])+["-GNinja"])
call(["ninja"])
