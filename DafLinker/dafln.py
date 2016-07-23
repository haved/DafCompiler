#!/usr/bin/python

from os.path import expandvars
from sys import argv
from os import getcwd, chdir
from os.path import join, split, isfile, isdir

defaultLinkfilePaths = ["/usr/share/daf/linker_search.txt", expandvars("$HOME/.daf/linker_search.txt")]
defaultLinkerType = "gnu_link_linux"
defaultLinkfile = "Linkfile"

exec_name = split(argv[0])[1]

class Argument:
    def __init__(self, text, file, line):
        self.text = text
        self.file = file
        self.line = line

class GnuLinkerLinux:
    def makeArgumentList(self, input):
        return cleanPaths([input.executable] + input.linker_args
        + splitTuples([("-L",libImport) for libImport in input.library_search])
        + input.getFilteredFilesAndLibs()
        + ["-o", input.outputFile])
    def parseArgument(self, args, index):
        pass
    def getName(self):
        return "gnu_link_linux"
    def getDefaultExecutable(self):
        return "ld"
    def getOnelineDesc(self):
        return "Using an ld command to link programs or shared libraries"
    def printHelpMessage(self):
        print("==="+self.getName()+"===",
        "example program: ld",
        "extra parameters:",
        "    -shared",
        "    -linkable <library>",
        "or:",
        "    -rpath <dir>",
        "desc: Links an executable or a shared library with all the input object files and libraries",
        "When linkng a shared library, the output is the soname of the library.",
        "By supplying `-linkable`, the output becomes a symlink to the linkable file.",
        "This is to maintain compatability with windows.",sep='\n')

class GppLinux:
    def __init__(self):
        self.shared = False
        self.linkable = None
        self.rpath = None

    def makeArgumentList(self, input):
        args = [input.executable] + cleanPaths(input.linker_args
        + splitTuples([("-L",libImport) for libImport in input.library_search])
        + input.getFilteredFilesAndLibs())

        output = input.outputFile
        if self.shared:
            args += ["-shared", "-fPIC"]
            if self.linkable != None:
                args += ["-Wl,-soname,"+split(self.outputFile)[1]]
                print("Make the symlink from " + self.outputFile + " pointing to " + self.linkable + "yourself!")
                output =  self.linkable
        elif self.linkable != None:
            logTotalError("Can't have a linkable output unless linking a shared library'")
            exit()
        if self.rpath:
            if self.shared:
                logTotalError("Can't link a shared library with an rpath! Aborting.'")
                exit()
            args.append("-Wl,-rpath,"+self.rpath)

        args += cleanPaths(["-o", output])

        return args
        
    def parseArgument(self, args, index):
        arg = args[index]
        argText = arg.text
        if argText == "-shared":
            self.shared = True
        elif argText == "-linkable":
            index += 1
            self.linkable = getFullName(getArg(args, index).text)
        elif argText == "-rpath":
            index += 1
            self.rpath = getArg(args, index).text
        else:
            return None
        return index
    def getName(self):
        return "g++_linux"
    def getDefaultExecutable(self):
        return "g++"
    def getOnelineDesc(self):
        return "Using g++ to link c++ programs or shared libraries"
    def printHelpMessage(self):
        print("==="+self.getName()+"===",
        "example program: g++",
        "extra parameters:",
        "    -shared",
        "    -linkable <library>",
        "or:",
        "    -rpath <dir>",
        "desc: Does exactly the same as gnu_link_linux, but includes stdc++ libraries",
        "May also order c++ libraries correctly for you",sep='\n')

linker_types = [GnuLinkerLinux(), GppLinux()]
def getLinkerType(text):
    for potential in linker_types:
        if potential.getName() == text:
            return potential
    return None
def showLinkerTypeHelp():
    print("dafln linker types:")
    for linkerType in linker_types:
        print("    ",(linkerType.getName()+":").ljust(20,' '),linkerType.getOnelineDesc(),sep='')
def showHelpPage():
    print("""Help page for dafln\nUsage: dafln <OPTION LIST>

List entries:
    -F <file>:              Load a linkfile
    <File>:                 Added as a file from a file search directory
    -l<library>:            Adds a library from a library search directory
    -I <dir>:               Add a search directory for files
    -L <dir>:               Add a search directory for linker libraries
    -A <dir>:               Adds a directory to the object file whitelist
    -o <file>:              Set output file name
    -X --help:              Print list of linker types
    -X <link_type> --help:  Print help for specific linker type
    -X <link_type> <exec>:  Set the linker type and what program to use
    -x <arg>:               Pass a single arg to the linker
    -h --help:              Show this help page""")
#
def log(file, line, level, text):
    if line > 0:
        print(file,":",line,": ",level,": ",text,sep='')
    else:
        print(file,": ",level,": ",text,sep='')
def logArg(arg, level, text):
    log(arg.file, arg.line, level, text)
def logError(arg, text):
    logArg(arg, "error", text)
def logWarning(arg, text):
    logArg(arg, "warning", text)
def logTotalError(text):
    log(exec_name, -1, "error", text)
#

def getFullName(file):
    return join(getcwd(), file)
def splitTuples(tuples):
    out = []
    for tuple in tuples:
        out += tuple
    return out

def cleanPaths(paths):
    wd = getFullName(".")[:-1]
    start = len(wd)
    return [path[start:] if path.startswith(wd) else path for path in paths]

def getArg(args, index):
    if index >= len(args):
        logError(args[index-1], "Expected something after " + args[index-1].text)
        exit(1);
    return args[index]
def appendUniqueDirectory(list, arg, listName):
    fullPath = getFullName(arg.text)
    if not isdir(fullPath):
        logWarning(arg, "The wanted " + listName + "'" + fullPath + "' doesn't exist'")
    elif fullPath in list:
        logWarning("The " + listName + " '"+ fullPath + "' is already added")
    else:
        list.append(fullPath)

class Input:
    def __init__(self):
        self.files = []
        self.file_search = []
        self.library_search = []
        self.whitelist = []
        self.whitelistLibraries = False
        self.outputFile = None
        self.linker_args = []
        self.type = None
        self.executable = None

    def makeArgumentList(self):
        return self.type.makeArgumentList(self)

    def addFile(self, arg):
        for dir in self.file_search+[getcwd()]:
            path = join(dir, arg.text)
            if path in self.files:
                logWarning(arg, "The file '"+arg.text+"' was already added")
            elif isfile(path):
                self.files.append(path)
                return True
        return False

    def addLibrary(self, arg):
        if arg.text in self.files:
            logWarning(arg, "The library '"+arg.text+"' was already added")
        else:
            self.files.append(arg.text)

    def addFileSearchDir(self, arg):
        appendUniqueDirectory(self.file_search, arg, "file search directory")

    def addLibrarySearchDir(self, arg):
        appendUniqueDirectory(self.library_search, arg, "library search directory")

    def addDirToWhitelist(self, arg):
        if(arg.text == "-l"):
            self.whitelistLibraries = True
        else:
            appendUniqueDirectory(self.whitelist, arg, "whitelist directory")

    def handleLinkerTypeChange(self, args, index):
        typeArg = getArg(args, index)
        if typeArg.text == "--help" or typeArg.text == "-h":
            showLinkerTypeHelp()
            exit()
        
        self.type = getLinkerType(typeArg.text)
        if self.type == None:
            logError(typeArg, "Linker type not found: ''",typeArg.text,"'. Aborting.")
            exit()

        index+=1
        programArg = getArg(args, index)
        if(programArg.text == "--help" or programArg.text == "-h"):
            self.type.printHelpMessage()
            exit()
        self.executable = programArg.text
        return index
    #
    def setDefaultValues(self):
        assert(self.type != None and self.executable != None)
        if self.outputFile == None:
            self.outputFile = "a.out"
    
    def getFilteredFilesAndLibs(self):
        if len(self.whitelist) == 0:
            return self.files
        out = []
        for file in self.files:
            if file.startswith("-l"):
                if self.whitelistLibraries:
                    out.append(file)
            else:
                for filter in self.whitelist:
                    if file.startswith(filter):
                        out.append(file)
                        break
        return out
    #
    def handleArguments(self, args):
        index = 0
        while index < len(args):
            arg = args[index]
            argText = arg.text
            if argText == "-F":
                index+=1
                handleFile(getArg(args, index).text, self)
            elif argText[:2] == "-l":
                self.addLibrary(arg)
            elif argText == "-I":
                index += 1
                self.addFileSearchDir(getArg(args, index))
            elif argText == "-L":
                index += 1
                self.addLibrarySearchDir(getArg(args, index))
            elif argText == "-A":
                index += 1
                self.addDirToWhitelist(getArg(args, index))
            elif argText == "-o":
                index += 1
                self.outputFile = getFullName(getArg(args, index).text)
            elif argText == "-X":
                index += 1
                index = self.handleLinkerTypeChange(args, index)
            elif argText == "-x":
                index += 1
                self.linker_args.append(getArg(args, index).text);
            elif argText == "-h" or argText == "--help":
                showHelpPage()
                exit()
            else:
                newIndex = self.type.parseArgument(args, index)
                if newIndex != None and newIndex >= 0:
                    index = newIndex
                else:
                    if not self.addFile(arg):
                        logError(arg, "The argument '"+arg.text+"' wasn't recognized as neither an option nor a file!")
            index += 1

def handleFile(filePath, input):
    args = []

    fileDir, fileName = split(getFullName(filePath))

    try:
        file = open(filePath)
        lineC = 0
        while True:
            lineC += 1
            line = file.readline()
            if len(line)==0:
                break
            for arg in line.split('#')[0].split('\n')[0].split(" "):
                if len(arg)>0:
                    args.append(Argument(arg, exec_name, lineC))
    except FileNotFoundError as e:
        return False
    if len(args) == 0:
        return True
    wd = getcwd()
    chdir(fileDir)
    input.handleArguments(args)
    chdir(wd)
    return True



from subprocess import call

def main():
    input = Input()
    input.type = getLinkerType(defaultLinkerType)
    input.executable = input.type.getDefaultExecutable()
    assert(input.type != None and input.executable != None)

    for linkerfile in defaultLinkfilePaths:
        handleFile(linkerfile, input)

    input.handleArguments([Argument(arg, exec_name, -1) for arg in argv[1:]])

    if len(input.files) == 0:
        if not "-F" in argv:
            if not handleFile("Linkfile", input):
                print("No input files, and no default Linkfile")
        else:
            print("No input files")

    input.setDefaultValues()
    args = input.makeArgumentList()

    print(exec_name,":",sep='',end=' ')
    for arg in args:
        print(arg, end=' ')
    print("")

    exit(call(args))

if __name__ == "__main__":
    main()
