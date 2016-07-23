#!/usr/bin/python

from os.path import expandvars
from sys import argv
from os import getcwd, chdir
from os.path import join, split, isfile, isdir

defaultLinkfilePaths = ["/usr/share/daf/linker_search.txt", expandvars("$HOME/.daf/linker_search.txt")]
defaultLinkerType = "gnu_link_linux"
defaultLinkfile = "Linkfile"

exec_name = argv[0]

class Argument:
    def __init__(self, text, file, line):
        self.text = text
        self.file = file
        self.line = line

class GnuLinkerLinux:
    def makeArgumentList(self, input):
        return [input.executable]
    def parseArgument(self, args, index):
        pass
    def getDefaultExecutable(self):
        return "ld"

def getLinkerType(text):
    if text == "gnu_link_linux":
        return GnuLinkerLinux()
    return None
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
#

def getArg(args, index):
    if index >= len(args):
        logError(args[index-1], "Expected something after " + args[index-1].text)
        exit(1);
    return args[index]
def getFullPath(arg):
    if isdir(arg.text):
        return join(getcwd(), arg.text)
    else:
        logWarning(arg, "Directory '"+join(getcwd(), arg.text)+"' not found'")
    return None
def appendIfReal(list, elm):
    if elm != None:
        list.append(elm)

class Input:
    def __init__(self):
        self.object_files = []
        self.libraries = []
        self.object_file_search = []
        self.library_search = []
        self.objWhitelist = []
        self.outputFile = None
        self.type = None
        self.executable = None

    def makeArgumentList(self):
        return self.type.makeArgumentList(self)

    def addObjectFile(self, arg):
        for dir in self.object_file_search:
            path = join(dir, arg.text)
            if isfile(path):
                self.object_files.append(path)
                return True
        if isfile(arg.text):
            self.object_files.append(join(getcwd(), arg.text))
            return True
        logError(arg, "Object file "+arg.text+" wasn't found in any search directory.")

    def addLibrary(self, arg):
        self.libraries.append(arg.text)

    def addObjectFileSearchDir(self, arg):
        appendIfReal(self.object_file_search, getFullPath(arg))

    def addLibrarySearchDir(self, arg):
        appendIfReal(self.library_search, getFullPath(arg))

    def addDirToObjectWhitelist(self, arg):
        appendIfReal(self.objWhitelist, getFullPath(arg))

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
                self.addObjectFileSearchDir(getArg(args, index))
            elif argText == "-L":
                index += 1
                self.addLibrarySearchDir(getArg(args, index))
            elif argText == "-A":
                index += 1
                self.addDirToObjectWhitelist(getArg(args, index))
            elif argText == "-o":
                index += 1
                self.outputFile = getArg(args, index)
            elif arg
            else:
                newIndex = self.type.parseArgument(args, index)
                if newIndex != None and newIndex >= 0:
                    index = newIndex
                else:
                    self.addObjectFile(arg)
            index += 1

def handleFile(filePath, input):
    args = []
    fileName = split(filePath)[1]

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
    chdir(split(filePath)[0])
    input.handleArguments(args)
    chdir(wd)
    return True



from subprocess import call

def main():
    input = Input()
    input.type = getLinkerType(defaultLinkerType)
    input.executable = input.type.getDefaultExecutable()
    assert(input.type != None)

    for linkerfile in defaultLinkfilePaths:
        handleFile(linkerfile, input)

    input.handleArguments([Argument(arg, exec_name, -1) for arg in argv[1:]])

    args = input.makeArgumentList()

    print(exec_name,":",sep='',end=' ')
    for arg in args:
        print(arg, end=' ')
    print("")

    exit(call(args))

if __name__ == "__main__":
    main()
