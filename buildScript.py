#!/usr/bin/env python3

from sys import argv
import os.path, os, subprocess

binaryName = "buildScript.py"
def fatal_error(text, *arg):
    print("{}: FATAL_ERROR: {}".format(binaryName, text.format(*arg)))
    exit(1)

def warning(text, *arg):
    print("{}: WARNING: {}".format(binaryName, text.format(*arg)))

def info(text, *arg):
    print("{}: INFO: {}".format(binaryName, text.format(*arg)))

def expectText(name, text, target):
    if text not in target:
        fatal_error("expected {} to be in [{}], not {}".format(name, ",".join(target), text))

#Key is option, value is default in debug
#List value means legal options, first item being default in debug
knownOptions = {"-buildDir": "DebugBuild",
                "-cmakeDir": "Compiler",
                "-allLLVM": ["false", "true"],
                "W": ["pedantic","none","all"],
                "O": ["none","0","1","2","s"],
                "g": ["true", "false"],
                "-release": False,
                "-extraCMakeArgs": "",
                "-extraMakeArgs": "-j3",
                "-extraCppFlags": "",
                "-extraLinkerFlags": "",
                "-tests": False,
                "-testFolder": "CompilerTests",
                "-testFilter": r"^\w+\.daf$",
                "-testDafOutput": "Testing/dafOutput.o",
                "-testCppFile": "Testing/dafMainCaller.cpp",
                "-testCppOutput": "Testing/dafMainCaller.o",
                "-testBinaryOutput": "Testing/outputBinary"}

releaseDefaults = {"-buildDir": "ReleaseBuild",
                   "-X86Only": "false",
                   "W": "all",
                   "O": "2",
                   "g": "false"}

class Options:
    def __init__(self):
        self.ops = {}

    def fatal_error_if_notAnOption(self, option):
        if option not in knownOptions:
            fatal_error("Unknown option: -{}", option)

    def setOption(self, name, value):
        if name in self.ops:
            warning("overriding option {} from {} to {}", name, self.ops[name], value)

        self.fatal_error_if_notAnOption(name)
        if isinstance(knownOptions[name], list): #We have to stick to it
            value = value.lower()
            expectText(name, value, knownOptions[name])

        self.ops[name] = value

    def needsValue(self, optionName):
        return knownOptions[optionName] != False

    def fillInDefaults(self):
        release = "-release" in self.ops and self.ops["-release"]

        if release:
            for key, val in releaseDefaults.items():
                if key not in self.ops:
                    self.setOption(key, val)

        for key, val in knownOptions.items():
            if key not in self.ops:
                defaultVal = val[0] if isinstance(val, list) else val
                self.setOption(key, defaultVal)

    def getOption(self, name):
        if name not in self.ops:
            print("Internal error, unknown option:", name)
            exit(1)
        return self.ops[name]

def parseCommandOptions(options, args):
    index = 0
    while index < len(args):
        opt = args[index]
        if opt in ["-h", "--help"]:
            printHelpMessage(knownOptions)
            exit(0)
        if len(opt)<1 or opt[0]!='-':
            fatal_error("expected option, not '{}'", opt)
        opt = opt[1:]
        index+=1
        options.fatal_error_if_notAnOption(opt)
        value = True #For things like -r and --tests
        if options.needsValue(opt):
            if index >= len(args):
                fatal_error("expected value after option {}", opt)
            value = args[index]
            index+=1
        options.setOption(opt, value)

def main():
    options = Options()
    parseCommandOptions(options, argv[1:])
    options.fillInDefaults()

    doCMake(options)

def tryDo(lamb):
    try:
        lamb()
    except e:
        print(e)
        exit(1)

def doCMake(options):
    buildDir = options.getOption("-buildDir")
    if not os.path.exists(buildDir):
        tryDo(lambda: os.makedirs(buildDir))

    cmakeDir = os.path.realpath(options.getOption("-cmakeDir"))
    if not os.path.exists(cmakeDir):
        fatal_error("Specified directory for CMakeLists.txt doesn't exist:", cmakeDir)

    cmakeCommand = ["cmake", cmakeDir]
    cmakeCommand += ["-DDAF_LINK_ALL_LLVM:BOOL=" + options.getOption("-allLLVM")]
    cmakeCommand += ["-DDAF_DEBUG_MACRO:BOOL=" + ("false" if options.getOption("-release") else "true")]
    cmakeCommand += ["-DDAF_EXTRA_CXX_FLAGS:STRING="+options.getOption("-extraCppFlags")]
    cmakeCommand += ["-DDAF_EXTRA_LINK_FLAGS:STRING="+options.getOption("-extraLinkerFlags")]
    cmakeCommand += options.getOption("-extraCMakeArgs").split()

    info("Running command: {}", "   ".join(cmakeCommand))

    with subprocess.Popen(cmakeCommand, cwd=buildDir) as cmakeCall:
        cmakeCall.wait()
        if cmakeCall.returncode is not 0:
            fatal_error("Cmake failed with error code {}", cmakeCall.returncode)

    

def printHelpMessage(knownOptions):
    default = [val for key,val in knownOptions.items()]
    default = [val[0] if isinstance(val, list) else val for val in default]

    print("""Usage: buildScript.py <options>
    Options:
    -h --help                Print this help message.
    --buildDir <buildDir>    Specify the folder in which we build. Default: "{0}"
    --cmakeDir <cmakeDir>    Specify the folder where CMakeLists.txt is. Default: "{1}"
    --allLLVM <true|false>   Link all LLVM targets, not just x86. Default: "{2}"
    -W <none|all|pedentic>   Print out warnings? Default:"{3}"
    -O <s|0|1|2|none>        Optimization level. Default: "{4}"
    -g <true|false>          Build with -g? Default: "{5}"
    --release                Changes a bunch of defaults and disables #define DAF_DEBUG
    --extraCMakeArgs         <string with extra CMake parameters> Default: "{7}"
    --extraMakeArgs          <string with extra make parameters> Default: "{8}"
    --extraCppFlags          <string with extra compiler flags> Default: "{9}"
    --extraLinkerFlags       <string with extra linker flags> Default: "{10}"

    --tests                           Enables testing of the newly compiled binary (if we got so far)
    --testFolder <testFolder>         Where to look for tests. Default: "{12}"
    --testFilter <testFilter>         Only test files that fit the given regex. Default: "{13}"
    --testDafOutput <testingDafO>     Specify where the .o from the DafCompiler is put. Default: "{14}"
    --testCppFile <testingCppFile>    Specify what C++ file is used to bootstrap daf code. Default: "{15}"
    --testCppOutput <testingCppO>     Specify where the C++ binary will be placed. Default:"{16}"
    --testBinaryOutput <binaryOutput> Specify where the binary will be put. Default: "{17}"
    """.format(*default))

if __name__ == "__main__":
    main()
