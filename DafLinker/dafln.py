#!/usr/bin/python
from os.path import expanduser

linker_search_files = ["/usr/share/daf/linker_search.txt", expanduser("~/.daf/linker_search.txt")]
maxLevel = 10

from sys import argv
from subprocess import call
from os.path import dirname, realpath, split, join
from os import chdir, getcwd

object_files = []
libraries = []
o_search_dirs = []
lib_search_dirs = []
output = None
linker = None
linker_args = []

def printHelpMessage():
    print(
 """Usage: dafln <OPTION LIST>

List entries:
    -F <file>:      Load a linkfile
    <A file>:       Added as an object file from a search directory.
    -l<library>:    Adds a library from a library search directory.
    -I <dir>:       Add a search directory for object files
    -L <dir>:       Add a search directory for linker libraries
    -o <file>:      Set output file name
    -X <linker>:    Set the linker program used (default: ld)
    -x <arg>:       Pass a single arg to the linker
    -rpath <dir>:   Set runtime shared library search directory
    -h --help       Print this help message
    """)
    exit();

def getArg(args, index):
    if index >= len(args):
        print(args[index-1][1]+":", str(args[index-1][2])+": error: expected something after", args[index-1][0])
        return None
    return args[index]

def addSearchDir(dir): #Only used for object file serch dirs
    o_search_dirs.append(dir[0])

def addLibrarySearchDir(dir):
    lib_search_dirs.append(dir[0])

def addObjectFile(arg):
    pass

def findAndHandleFile(arg, level):
    if handleFile(arg[0], level) == False:
        print(arg[1]+":", str(arg[2])+": error: File not found:", arg[0])

def handleFile(filePath, level):
    if level > maxLevel:
        print("Aborting: Already", level, "levels in. Recursion?")
        exit()
    wd = getcwd()
    filePath = join(wd, filePath)
    args = []
    try:
        file = open(filePath);
        fileName = split(filePath)[1]
        lineNum = 0
        while True:
            lineNum+=1
            line = file.readline()
            if len(line) == 0:
                break;

            for arg in line.split('#')[0].split('\n')[0].split(' '):
                if len(arg) > 0:
                    args.append((arg, fileName, lineNum))
        file.close()
    except FileNotFoundError as e:
        return False;
    chdir(dirname(filePath))
    handleParameters(args, level)
    chdir(wd)
    return True

def handleParameters(args, level):
    i = 0
    while i < len(args):
        arg = args[i][0]
        if arg == "--help":
            printHelpMessage()
        elif arg == "-F":
            i+=1
            findAndHandleFile(getArg(args, i), level+1)
        elif arg == "-I":
            i+=1
            addSearchDir(getArg(args, i))
        elif arg == "-L":
            i+=1
            addLibrarySearchDir(getArg(args, i))
        elif arg[:2] == "-l":
            libraries.append(arg)
        elif arg == "-o":
            i+=1
            newOut = getArg(args, i)
            if output != None:
                print(newOut[1]+":", str(newOut[2])+": warning: Setting output name again! (from '", output, "' to '", newOut[0], "')")
            output = newOut[0]
        else:
            addObjectFile(args[i])
        i+=1

def main() :
    global object_files, libraries, lib_search_dirs, output, linker, linker_args

    for linker_search in linker_search_files:
        handleFile(linker_search, 0)

    if output == None:
        output = "daf.out"
    if linker == None:
        linker = "ld"

    args = [linker]+linker_args
    for dir in lib_search_dirs:
        args+=["-L", dir]
    args += object_files+libraries+["-o", output]
    
    for arg in args:
        print(arg, end=" ")
    print("")

    retcode = call(args)
    exit(retcode)

if __name__ == "__main__":
    main()