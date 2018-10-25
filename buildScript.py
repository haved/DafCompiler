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

def verbose_log(text, *arg):
    print("{}: VERBOSE: {}".format(binaryName, text.format(*arg)))


def printHelpText():
    print("""\
Runs the commands used to build the daf compiler.
Can optionally setup opam before attempting to compile.
Can optionally run basic compiler tests with the binary.
Usage: ./buildScript.py <options>
    Options:
    --verbose                Prints all commands executed

    --opam_setup             Switches compiler version and installs required opam packages before compiling

    --tests                  Run tests
    --testFolder             Specify folder in which to look for tests. Default: OCompilerTests
    --testFilter <filter>    Specify a regex filtering file names for testing. Default: .daf files
    --dafc_stdout            Print dafc stdout
    --test_stdout            Print stdout from the tests

    --help                   Print this help message
    """)

compiler_folder = "OCompiler"
relative_binary_path = "{}/dafc_main.native".format(compiler_folder)

verbose = False
run_opam_setup = False
run_tests = False
test_folder = "OCompilerTests"
test_filter = "^.+\\.daf$"
forward_dafc_stdout = False
forward_test_stdout = False
test_options_set = False

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
    global verbose, run_opam_setup, run_tests, test_folder, test_filter, forward_dafc_stdout, forward_test_stdout, test_options_set
    argsLeft = args

    while len(argsLeft):
        arg = argsLeft[0]

        if arg == '--verbose':
            verbose = True
        elif arg == '--setup_opam':
            run_opam_setup = True
        elif arg == '--tests':
            run_tests = True
        elif arg == '--testFolder':
            test_folder = nextArg(argsLeft)
            test_options_set = True
            argsLeft = argsLeft[1:]
        elif arg == '--testFilter':
            test_filter = nextArg(argsLeft)
            test_options_set = True
            argsLeft = argsLeft[1:]
        elif arg == '--dafc_stdout':
            forward_dafc_stdout = True
            test_options_set = True
        elif arg == '--test_stdout':
            forward_test_stdout = True
            test_options_set = True
        elif arg == '--help':
            printHelpText()
            exit(0)
        else:
            fatal_error("Unrecognized option: {}", arg)
        argsLeft = argsLeft[1:]

def main():
    parseOptions(argv[1:])
    if run_opam_setup:
        fatal_error("The opam setup command is a lie, for now")
    doMake()
    if run_tests:
        binary_path = os.path.join(os.getcwd(), relative_binary_path)
        doTests(binary_path)
    elif test_options_set:
        warning("Command arguments related to tests were given, but we aren't running tests")

dir_stack = []
def pushdir(dir):
    dir_stack.append(os.getcwd())
    if verbose:
        verbose_log("Changing dir: {}", dir)
    try:
        os.chdir(dir)
    except FileNotFoundError as e:
        fatal_error("The directory {} doesn't exist", os.path.join(os.getcwd(), dir))

def popdir():
    dir = dir_stack.pop()
    if verbose:
        verbose_log("Returning to dir: {}", dir)
    os.chdir(dir)

def Popen(arg_list, **dic):
    if verbose:
        def capture(text):
            text = text.replace('"', '\\"');
            return '"'+text+'"' if " " in text else text
        verbose_log(" ".join([capture(arg) for arg in arg_list]))
    return subprocess.Popen(arg_list, **dic)

def doMake():
    info("Compiling dafc")
    pushdir(compiler_folder)
    with Popen(["make", "-j3"]) as makeCall:
        makeCall.wait()
        if makeCall.returncode is not 0:
            fatal_error("Make failed with return code {}", makeCall.returncode)
    popdir()

def doTests(binary_name):
    pushdir(test_folder)

    filter = re.compile(test_filter)

    files_to_test = [f_name for f_name in os.listdir('.') if filter.match(f_name)]
    tests_failed_count = 0

    for index, f_name in enumerate(files_to_test, 1):
        info("Running test {}/{}: {}", index, len(files_to_test), f_name)
        with Popen([binary_name, f_name], stdout=None if forward_dafc_stdout else subprocess.PIPE) as comp:
            comp.wait(timeout=10)
            if comp.returncode is not 0:
                error("Test {}/{} failed: return code {}", index, len(files_to_test), comp.returncode)
                tests_failed_count += 1

    info("{}/{} tests passed. {} failed", len(files_to_test)-tests_failed_count, len(files_to_test), tests_failed_count)

    popdir()


if __name__ == "__main__":
    main()

