#!/usr/bin/python
from os.path import expanduser

linker_search_files = ["/usr/share/daf/linker_search.txt", expanduser("~/.daf/linker_search.txt")]

from sys import argv
from subprocess import call
from os.path import dirname, realpath, split
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
        print(args[index-1][1]+":", args[index-1][2]+": error: expected something after", args[index-1][0])
        return None
    return args[index]

def addSearchDir(dir): #Only used for object file serch dirs
    o_search_dirs.append(dir[0])

def addLibrarySearchDir(dir):
    lib_search_dirs.append(dir[0])

def addObjectFile(arg):
    pass

def handleFile(filePath):
    try:
        file = open(filePath);
        fileName = split(filePath)[1]
        args = []
        lineNum = 0
        while True:
            lineNum+=1
            line = file.readline()
            if len(line) == 0:
                break;

            for arg in line.split('#')[0].split('\n')[0].split(' '):
                if len(arg) > 0:
                    args.append((arg, fileName, lineNum))
            
        chdir(dirname(filePath))
        handleParameters(args)
    except FileNotFoundError as e:
        return False;
    return True;

def handleParameters(args):
    i = 0
    while i < len(args):
        arg = args[i][0]
        if arg == "--help":
            printHelpMessage()
        elif arg == "-I":
            i+=1
            addSearchDir(getArg(args, i))
        elif arg == "-L":
            i+=1
            addLibrarySearchDir(getArg(args, i))
        else:
            addObjectFile(args[i])
        i+=1

def main() :
    global object_files, libraries, lib_search_dirs, output, linker, linker_args

    main_wd = getcwd();
    for linker_search in linker_search_files:
        handleFile(linker_search)
    chdir(main_wd) #To make the output happen at cwd

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

if __name__ == "__main__":
    main()