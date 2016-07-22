#!/usr/bin/python

from os.path import expandvars
from sys import argv

defaultLinkfilePaths = ["/usr/share/daf/linker_search.txt", expandvars("$HOME/.daf/linker_search.txt")]
defaultLinkerType = "gnu_link_linux"
defaultLinkfile = "Linkfile"

exec_name = argv[0]

class Input:
    def __init__(self):
        self.object_files = []
        self.libraries = []
        self.object_file_search = []
        self.library_search = []
        self.outputFile = None
        self.type = None
        self.executable = None
    
    def makeArgumentList(self):
        return self.type.makeArgumentList(self)

class Parameter:
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

from os import getcwd, chdir
from os.path import join, split

def handleFile(filePath, input):
    params = []
    try:
        file = open(filePath)
        while True:
            line = file.readline()
            if len(line)==0:
                break
            params += line.split('#')[0].split('\n')[0].split(" ")
    except FileNotFoundError as e:
        return False
    wd = getcwd()
    chdir(split(join(wd,filePath))[0])
    handleParameters(params, input)
    chdir(wd)
    return True

def handleParameters(parameters, input):
    pass


from subprocess import call

def main():
    input = Input()
    input.type = getLinkerType(defaultLinkerType)
    input.executable = input.type.getDefaultExecutable()
    assert(input.type != None)
       
    for linkerfile in defaultLinkfilePaths:
        handleFile(linkerfile, input)

    handleParameters([Parameter(arg, exec_name, -1) for arg in argv[:-1]], input)

    args = input.makeArgumentList()

    print(exec_name,":",sep='',end=' ')
    for arg in args:
        print(arg, end=' ')
    print("")

    exit(call(args))

if __name__ == "__main__":
    main()