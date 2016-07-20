#!/usr/bin/python
from os.path import expanduser

linker_search_files = ["/usr/share/daf/linker_search.txt", expanduser("~/.daf/linker_search.txt")]
maxLevel = 10

from sys import argv
from subprocess import call
from os.path import dirname, realpath, split, join, isfile
from os import chdir, getcwd

object_files = []
libraries = []
o_search_dirs = []
lib_search_dirs = []
output = None
linker = None
linker_args = []
rpath = None

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
        exit(-1)
    return args[index]

def addSearchDir(dir): #Only used for object file serch dirs
    wd = getcwd()
    o_search_dirs.append(join(wd, dir[0]))

def addLibrarySearchDir(dir):
    wd = getcwd()
    lib_search_dirs.append(join(wd, dir[0]))

def addObjectFile(arg):
    name = arg[0]
    for dir in o_search_dirs:
        path = join(dir, name)
        if isfile(path):
            object_files.append(path)
            return
    path = join(getcwd(), name)
    if isfile(path):
        object_files.append(path)
    else:
        print(arg[1]+":",str(arg[2])+": Object file", name, "not found in any search directory or in cwd")

def findAndHandleFile(arg, level):
    if handleFile(arg[0], level) == False:
        print(arg[1]+":", str(arg[2])+": error: File not found:", arg[0])

def handleFile(filePath, level):
    wd = getcwd()
    filePath = join(wd, filePath)
    fileName = split(filePath)[1]
    if level > maxLevel:
        print(fileName+": aborting: Already", level, "levels in. Recursion?")
        exit()
    args = []
    try:
        file = open(filePath);
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
        if arg == "-h" or arg == "--help":
            printHelpMessage()
        elif arg == "-F":
            i+=1
            findAndHandleFile(getArg(args, i), level+1)
        elif arg[:2] == "-l":
            libraries.append(arg)
        elif arg == "-I":
            i+=1
            addSearchDir(getArg(args, i))
        elif arg == "-L":
            i+=1
            addLibrarySearchDir(getArg(args, i))
        elif arg == "-o":
            i+=1
            newOut = getArg(args, i)
            if output != None:
                print(newOut[1]+":", str(newOut[2])+": warning: Setting output name again! (from '", output, "' to '", newOut[0], "')")
            output = newOut[0]
        elif arg == "-X":
            i+=1
            newLinker = getArg(args, i)
            if linker != None:
                print(newLinker[1]+":", str(newLinker[2])+": warning: Setting linker again! (from '", linker, "' to '", newLinker[0], "')")
            linker = newLinker[0]
        elif arg == "-x":
            i+=1
            linker_args.append(getArg(args, i)[0])
        elif arg == "-rpath":
            i+=1
            newRpath = getArg(args, i)
            if rpath != None:
                print(newRpath[1]+":", str(newRpath[2])+": warning: Setting rpath again! (from '", rpath, "' to '", newRpath[0], "')")
            rpath = newRpath[0]
        else:
            addObjectFile(args[i])
        i+=1

def main() :
    global object_files, libraries, lib_search_dirs, output, linker, linker_args, rpath

    for linker_search in linker_search_files:
        handleFile(linker_search, 0)

    handleParameters([(arg, "dafln", 0) for arg in argv][1:], 0)

    if output == None:
        output = "daf.out"
    if linker == None:
        linker = "ld"

    args = [linker]+linker_args
    if rpath != None:
        args += ["-rpath", rpath]
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