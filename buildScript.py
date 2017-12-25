#!/usr/bin/env python3

from sys import argv
import os.path, os, subprocess, re

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

def cleanupTmpFiles(options, silent=False):
    remove = [options.getOption("-testDafOutput"), options.getOption("-testCppOutput"), options.getOption("-testBinaryOutput")]
    if not silent:
        info("Removing {}, {} and {}", *remove)
    for file in remove:
        try:
            os.remove(file)
        except FileNotFoundError as e:
            if not silent:
                info("File didn't exist: {}", file)


#Key is option, value is default in debug
#List value means legal options, first item being default in debug
knownOptions = {"-buildDir": "DebugBuild",
                "-cmakeDir": "Compiler",
                "-onlyX86": ["true", "false"],
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
                "-testDafOutput": "CompilerTests/Testing/dafOutput.o",
                "-testCppFile": "CompilerTests/Testing/dafMainCaller.cpp",
                "-testCppOutput": "CompilerTests/Testing/dafMainCaller.o",
                "-testBinaryOutput": "CompilerTests/Testing/outputBinary",
                "-outputTesting": False}

releaseDefaults = {"-buildDir": "ReleaseBuild",
                   "-onlyX86": "false",
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
    doMake(options)

    if options.getOption("-tests"):
        compileDafCaller(options)
        doTests(options)


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
    cmakeCommand += ["-DDAF_TARGET_ONLY_x86:BOOL=" + options.getOption("-onlyX86")]
    cmakeCommand += ["-DDAF_DEBUG_MACRO:String=" + ("false" if options.getOption("-release") else "true")]
    extraCppFlags = [options.getOption("-extraCppFlags")]
    O_opt = options.getOption("O")
    if O_opt != "none":
        extraCppFlags += ["-O"+O_opt]
    if options.getOption("g") == "true":
        extraCppFlags += ["-g"]
    warn_opt = options.getOption("W")
    if warn_opt in ["all", "pedantic"]:
        extraCppFlags += ["-Wall", "-Weffc++"]
        if warn_opt == "pedantic":
            extraCppFlags += ["-Wextra", "-pedantic"]
    cmakeCommand += ["-DDAF_EXTRA_CXX_FLAGS:STRING="+" ".join(extraCppFlags)]
    cmakeCommand += ["-DDAF_EXTRA_LINK_FLAGS:STRING="+options.getOption("-extraLinkerFlags")]
    cmakeCommand += options.getOption("-extraCMakeArgs").split()

    info("Running command: {}", "   ".join(cmakeCommand))

    with subprocess.Popen(cmakeCommand, cwd=buildDir) as cmakeCall:
        cmakeCall.wait()
        if cmakeCall.returncode is not 0:
            fatal_error("Cmake failed with error code {}", cmakeCall.returncode)

def doMake(options):
    buildDir = options.getOption("-buildDir")
    if not os.path.exists(buildDir):
        fatal_error("Missing build dir all of a sudden");

    makeCommand = ["make"]
    makeCommand += options.getOption("-extraMakeArgs").split()

    info("Running command: {}", "   ".join(makeCommand))

    with subprocess.Popen(makeCommand, cwd=buildDir) as makeCall:
        makeCall.wait()
        if makeCall.returncode is not 0:
            fatal_error("Make failed with error code {}", makeCall.returncode)


def compileDafCaller(options):
    compileCommand = ["g++", "-c", options.getOption("-testCppFile"),
                      "-o", options.getOption("-testCppOutput"), "-std=c++14", "-Wall", "-Wextra", "-g"]

    info("Running command: {}", "   ".join(compileCommand))

    with subprocess.Popen(compileCommand) as compileCall:
        compileCall.wait()
        if compileCall.returncode is not 0:
            fatal_error("Compiling C++ main file failed with error code: {}", compileCall.returncall)

def runCommand(name, command, timeout, forwardOutput=False):
    try:
        with subprocess.Popen(command, stdout=None if forwardOutput else subprocess.PIPE, stderr=None if forwardOutput else subprocess.PIPE) as process:
            process.wait(timeout=timeout)
            if process.returncode is not 0:
                info("last command: {}", "   ".join(command))
                if not forwardOutput:
                    info("STDOUT for this command: ")
                    print(process.stdout.read().decode("utf-8"))
                    info("STDERR for this command: ")
                    print(process.stderr.read().decode("utf-8"))
                fatal_error("{} failed with return code {}", name, process.returncode)
    except TimeoutError as e:
        info("last command: {}", "   ".join(command))
        fatal_error("{} timed out", name)

def doTests(options):
    filesToConsider = []

    topFolder = options.getOption("-testFolder")
    def pat(file):
        return os.path.join(topFolder, file)

    def dig(folder, filesToConsider):
        list = [os.path.join(folder, item) for item in os.listdir(pat(folder))]
        filesToConsider += [item for item in list if os.path.isfile(pat(item))]
        for dir in [item for item in list if os.path.isdir(pat(item))]:
            dig(dir, filesToConsider)

    dig("", filesToConsider)

    filter = re.compile(options.getOption("-testFilter"))
    filesToConsider = [file for file in filesToConsider if filter.match(file)]

    info("Testing {} files", len(filesToConsider))

    for index, file in enumerate(filesToConsider):
        testString = "({}/{}): {}".format(index+1, len(filesToConsider), file)
        info("Testing {}", testString)
        dafCompileCommand = [os.path.join(options.getOption("-buildDir"),"DafCompiler")]
        dafCompileCommand += [os.path.join(options.getOption("-testFolder"), file)]
        dafCompileCommand += ["-o", options.getOption("-testDafOutput")]
        runCommand("DafCompiler on test {}".format(testString), dafCompileCommand, 10, options.getOption("-outputTesting"))

        linkCommand = ["g++", options.getOption("-testDafOutput"), options.getOption("-testCppOutput")]
        linkCommand += ["-o", options.getOption("-testBinaryOutput")]
        runCommand("Linking on test {}".format(testString), linkCommand, 10, True)

        runFileCommand = [options.getOption("-testBinaryOutput")]
        runCommand("Running test {}".format(testString), runFileCommand, 10, options.getOption("-outputTesting"))

    info("All {} tests successful", len(filesToConsider))
    cleanupTmpFiles(options)

def printHelpMessage(knownOptions):
    default = [val for key,val in knownOptions.items()]
    default = [val[0] if isinstance(val, list) else val for val in default]

    print("""Usage: buildScript.py <options>
    Options:
    -h --help                Print this help message.
    --buildDir <buildDir>    Specify the folder in which we build. Default: "{0}"
    --cmakeDir <cmakeDir>    Specify the folder where CMakeLists.txt is. Default: "{1}"
    --onlyX86 <true|false>   Build with only x86 target. Default: "{2}"
    -W <none|all|pedentic>   Print out warnings? Default:"{3}"
    -O <s|0|1|2|none>        Optimization level. Default: "{4}"
    -g <true|false>          Build with -g? Default: "{5}"
    --release                Changes a bunch of defaults and builds without #define DAF_DEBUG
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
    --outputTesting                   Print output from the compiler and the test programs. Default: "{18}"
    """.format(*default))

if __name__ == "__main__":
    main()
