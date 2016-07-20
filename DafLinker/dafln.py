#!/usr/bin/python
from os.path import expanduser

linker_search_files = ["/usr/share/daf/linker_search.txt", expanduser("~/.daf/linker_search.txt")]

from sys import argv
from subprocess import call
from os.path import dirname, realpath
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
    -F <file>:   Load a linkfile
    <A file>:    Added as an object file from a search directory.
    -l<library>: Adds a library from a search directory.
    -I <dir>:    Add a search directory for object files
    -L <dir>:    Add a search directory for linker libraries
    -o <file>:   Set output file name
    -X <linker>: Set the linker program used
    -x <arg>:    Pass a single arg to the linker
    -h --help    Print this help message
    """)

def handleFile(filePath):
    try:
        file = open(filePath);
        args = []
        while True:
            line = file.readline()
            if len(line) == 0:
                break;

            for arg in line.split('\n')[0].split(' '):
                if len(arg) > 0:
                    args.append(arg)
            
        chdir(dirname(filePath))
        handleParameters(args)
    except FileNotFoundError as e:
        return False;
    return True;

def handleParameters(args):
    i = 0
    while i < len(args):
        arg = args[i]
        if arg == "--help":
            printHelpMessage()

def main() :
    global output, linker, linker_args, object_files, libraries

    main_wd = getcwd();
    for linker_search in linker_search_files:
        handleFile(linker_search)
    chdir(main_wd) #To make the output happen at cwd

    if output == None:
        output = "daf.out"
    if linker == None:
        linker = "ld"

    args = [linker]+linker_args
    for dir in lib_dirs:
        args+=["-L", dir]
    args += object_files+libraries+["-o", output]
    print(args);

    retcode = call(args)

if __name__ == "__main__":
    main()