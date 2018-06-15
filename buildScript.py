#!/usr/bin/env python3

from sys import argv
import os.path, os, subprocess, re
from shutil import rmtree

binaryName = "buildScript.py"
def fatal_error(text, *arg):
    print("{}: FATAL_ERROR: {}".format(binaryName, text.format(*arg)))
    exit(1)

def warning(text, *arg):
    print("{}: WARNING: {}".format(binaryName, text.format(*arg)))

def info(text, *arg):
    print("{}: INFO: {}".format(binaryName, text.format(*arg)))


def printHelpText():
    print("""Usage: ./buildScript.py <options>
    Options:

    --tests                  Run tests
    --testFolder             Specify folder in which to look for tests. Default: OCompilerTests
    --testFilter <filter>    Specify a regex filtering file names for testing
    --dafc_stdout:bool       Should dafc stdout be printed? Default: No
    --test_stdout:bool       Should test stdout be printed? Default: No

    --help                   Print this help message
    """)

compiler_folder = "OCompiler"
relative_binary_path = "{}/dafc_main.native".format(compiler_folder)

run_tests = False
test_folder = "OCompilerTests"
test_filter = "^.+\\.daf$"
forward_dafc_stdout = False
forward_test_stdout = False

def nextArg(args):
    if len(args) < 2:
        fatal_error("Expected an option after: {}", args[0])
    return args[1]

def parseBool(arg):
    if arg in ['yes', 'Yes', 'YES', 'y', 'Y', 'true', 'True', 'TRUE', 't', 'T']:
        return True
    elif arg in ['no', 'No', 'NO', 'n', 'N', 'false', 'False', 'FALSE', 'f', 'F', 'nil']:
        return False
    fatal_error("Failed parsing bool: {}", arg)

def parseOptions(args):
    global run_tests, test_folder, test_filter, forward_dafc_stdout, forward_test_stdout
    argsLeft = args

    while len(argsLeft):
        arg = argsLeft[0]

        if arg == '--tests':
            run_tests = True
        elif arg == '--testFolder':
            test_folder = nextArg(argsLeft)
            argsLeft = argsLeft[1:]
        elif arg == '--help':
            printHelpText()
            exit(0)
        elif arg == '--testFilter':
            test_filter = nextArg(argsLeft)
            argsLeft = argsLeft[1:]
        elif arg == '--dafc_stdout':
            forward_dafc_stdout = parseBool(nextArg(argsLeft))
            argsLeft = argsLeft[1:]
        elif arg == '--test_stdout':
            forward_test_stdout = parseBool(nextArg(argsLeft))
            argsLeft = argsLeft[1:]
        else:
            fatal_error("Unrecognized option: {}", arg)
        argsLeft = argsLeft[1:]

def main():
    parseOptions(argv[1:])
    doMake()
    if run_tests:
        binary_path = os.path.join(os.getcwd(), relative_binary_path)
        doTests(binary_path)

def doMake():
    prev_cwd = os.getcwd()
    os.chdir(compiler_folder)
    info("Compiling dafc")
    with subprocess.Popen(["make", "-j3"]) as makeCall:
        makeCall.wait()
        if makeCall.returncode is not 0:
            fatal_error("Make failed with return code {}", makeCall.returncode)
    os.chdir(prev_cwd)

def doTests(binary_name):
    prev_cwd = os.getcwd()
    os.chdir(test_folder)

    filter = re.compile(test_filter)

    files_to_test = [f_name for f_name in os.listdir('.') if filter.match(f_name)]

    for index, f_name in enumerate(files_to_test, 1):
        info("Running test {}/{}: {}", index, len(files_to_test), f_name)
        with subprocess.Popen([binary_name, f_name], stdout=None if forward_dafc_stdout else subprocess.PIPE) as comp:
            comp.wait(timeout=10)
            if comp.returncode is not 0:
                fatal_error("Test {}/{} failed: return code {}", index, len(files_to_test), comp.returncode)

    os.chdir(prev_cwd)


if __name__ == "__main__":
    main()

