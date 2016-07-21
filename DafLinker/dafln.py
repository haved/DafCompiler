#!/usr/bin/python
from os.path import expanduser
from sys import argv

exec_name = argv[0] #Whatever is used to call this
version = "1.0"
linker_search_files = ["/usr/share/daf/linker_search.txt", expanduser("~/.daf/linker_search.txt")]
maxLevel = 10
defaultFile = "Linkfile"
defaultLinker = "ld"
defaultOutput = "a.out"
defaultPacker = "ar"

exec_name_colon = exec_name+":"

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
static = False
shared = False
soname = None

triedAddingOF = False

def printHelpMessage():
    print("Help page for dafln v.", version+"\n"+
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

def log(arg, text):
    print(arg[1]+":", str(arg[2]) + text)

def logWarning(arg, text):
    log(arg,"warning: " + text)

def logError(arg, text):
    log(arg, "error: " + text)

def getArg(args, index):
    if index >= len(args):
        logError(args[index-1], "expected something after " + args[index-1][0])
        exit(-1)
    return args[index]

def addSearchDir(dir): #Only used for object file serch dirs
    fullPath = join(getcwd(), dir[0])
    if fullPath in o_search_dirs:
        logWarning(dir, "Added the object file search directory '"+fullPath+"' a second time")
    else:
        o_search_dirs.append(fullPath)

def addLibrarySearchDir(dir):
    fullPath = join(getcwd(), dir[0])
    if fullPath in lib_search_dirs:
        logWarning(dir, "Added the library search directory '"+fullPath+"' a second time")
    else:
        lib_search_dirs.append(fullPath)

def addObjectFile(arg):
    triedAddingOF = True
    name = arg[0]
    for dir in o_search_dirs+[getcwd()]:
        path = join(dir, name)
        if isfile(path):
            if path in object_files:
                logError(arg, "Object file '" + name + "' was already registered")
            else:
                object_files.append(path)
            return
    
    logWarning(arg, "Object file'" + name + "'not found in any search directory or in cwd")

def addLibrary(arg):
    if arg[0] in libraries:
        logWarning(arg, "Added library '"arg[0]"' a second time")
    else:
        libraries.append(arg[0])

def findAndHandleFile(arg, level):
    if handleFile(arg[0], level) == False:
        logError(arg, "File not found: " + arg[0])

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
            addLibrary(args[i])
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
                logWarning(newOut, "Setting output name again! (from '" + output + "' to '" + newOut[0] + "')")
            output = newOut[0]
        elif arg == "-X":
            i+=1
            newLinker = getArg(args, i)
            if linker != None:
                logWarning(newLinker, "Setting linker again! (from '" + linker + "' to '" + newLinker[0] + "')")
            linker = newLinker[0]
        elif arg == "-x":
            i+=1
            linker_args.append(getArg(args, i)[0])
        elif arg == "-rpath":
            i+=1
            newRpath = getArg(args, i)
            if rpath != None:
                logWarning(newRpath, "Setting rpath again! (from '" + rpath + "' to '" + newRpath[0] + "')")
            rpath = newRpath[0]
        elif arg == "-static":
            static = True
            if shared:
                logWarning(args[i], "Changed library to static when already set to shared!")
                shared = False
        elif arg == "-shared":
            shared = True
            if static:
                logWarning(args[i], "Changed library to shared when already set to static!")
                static = False
        elif arg == "-soname":
            i+=1
            soname = getArg(args, i)[0]
        else:
            addObjectFile(args[i])
        i+=1

def main() :
    global object_files, libraries, lib_search_dirs, output, linker, linker_args, rpath, static, shared, soname, triedAddingOF

    for linker_search in linker_search_files:
        handleFile(linker_search, 0)
    
    handleParameters([(arg, exec_name, 0) for arg in argv[1:]], 0)

    if(not triedAddingOF):
        if not handleFile(defaultFile, 0):
            print(exec_name_colon, "No input files specified, and no Linkfile found")
            exit()

    if len(object_files) == 0:
        print(exec_name_colon, "No input files")
        exit()

    if output == None:
        output = defaultOutput
    if linker == None:
        linker = defaultLinker

    if soname != None and not shared:
        print(exec_name_colon, "error: Can't handle soname unless linking a shared library")
        exit()

    args = []
    if static:
        for lib in libraries:
            print(exec_name_colon, "warning: Skipping library:", lib)
        if rpath != None:
            print(exec_name_colon, "warning: Skipping rpath:", rpath)
        args = [defaultPacker, "rvs", output] + object_files;
    else:
        args = [linker]+linker_args
        if shared:
            args += ["-shared", "-fPIC"]
            if soname:
                args += ["-soname", soname]
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