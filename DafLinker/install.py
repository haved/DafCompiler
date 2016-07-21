#!/usr/bin/python

binarySrc = "./dafln.py"
binaryOut = "dafln"
settingsOut = "linker_search.txt"
binLoc = "/usr/bin"
setLoc = "/usr/share/daf"

from sys import argv
from os.path import join, isfile
from os import chmod

args = argv[1:]
exec_name = argv[0]

installBin = False
installSet = False
uninstall  = False

if len(args) == 0:
    args = ["--help"]

index = 0
while index < len(args):
    arg = args[index]
    if arg == "-h" or arg == "--help":
        print(
"""Installer for dafln, the daf linker
Usage: ./install.py <options>
Options:
    -rb <path>      Set binary install location (default: /usr/bin/)
    -rs <path>      Set settings location (default: /usr/share/daf/)
    -h --help:      Print this help message

    -I:     Install binary file
    -S:     Install settings file (if not already there)
    -IS:    Install both
    -U:     Uninstall both
""")
        exit();
    elif arg == "-rb":
        index += 1
        if index >= len(args):
            print(exec_name+": error: Expected path after '-rb'")
            exit()
        binLoc = args[index]
    elif arg == "-rs":
        index += 1
        if index >= len(args):
            print(exec_name+": error: Expected path after '-rs'")
            exit()
        setLoc = args[index]
    elif arg == "-I"
        installBin = True
    elif arg == "-S"
        installSet = True
    elif arg == "-IS":
        installBin = True
        installSet = True
    elif arg = "-U":
        uninstall = True
    index+=1

if uninstall and (installBin or installSet):
    print("Both installing and uninstalling. If you want to reinstall, run uninstall first, then install")
    exit()

from subprocess import call

if installBin:
    binPath = join(binLoc, binaryOut)
    print("Installing binary to: ", binPath)
    try:
        src = open(binarySrc, mode='rb')
        out = open(binPath,   mode='wb')

        out.write(src.read())
        chmod(out, "+x")

        src.close()
        out.close()
    except FileNotFoundError as e:
        print("Something went wrong when installing binary. Root access?")
if installSet:
    setPath = join(setLoc, settingsOut)
    if isfile(setPath):
        print(setPath, "already exists")
    else:
        print("Installing settings to: ", setPath)
        try:
            file = open(setPath, mode='w')
            print("-L lib", file=file)
            file.close()
        except FileNotFoundError as e:
            print("Something went wrong when installing settings. Root access?")
if uninstall:
    binPath = join(binLoc, binaryOut)
    setPath = join(setLoc, settingsOut)
    call(["rm", binPath, setPath])
