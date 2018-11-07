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
Can optionally run basic compiler tests with the binary.
Usage: ./buildScript.py <options>
    Options:
    --verbose                Prints all commands executed

    --tests                  Run tests
    --testFolder             Specify folder in which to look for tests. Default: OCompilerTests
    --testFilter <filter>    Specify a regex filtering file names for testing. Default: .daf files
    --dafc_stdout            Print dafc stdout
    --test_stdout            Print stdout from the tests

    --help                   Print this help message then exit
    --opam_setup             Run through the interactive setup of opam then exit
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
        elif arg == '--opam_setup':
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
        opam_setup()
        exit(0)
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

def command_to_string(arg_list):
    def capture(text):
        text = text.replace('"', '\\"');
        return '"'+text+'"' if " " in text else text
    return " ".join([capture(arg) for arg in arg_list])

def Popen(arg_list, **dic):
    if verbose:
        verbose_log(command_to_string(arg_list))
    return subprocess.Popen(arg_list, **dic)

def Popen_opam(arg_list, **dic):
    return Popen(opam_exec_command(arg_list), **dic)

def doMake():
    info("Compiling dafc")
    pushdir(compiler_folder)
    with Popen_opam(["make", "-j3"]) as makeCall:
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
                warning("Test {}/{} failed: return code {}", index, len(files_to_test), comp.returncode)
                tests_failed_count += 1

    info("{}/{} tests passed. {} failed", len(files_to_test)-tests_failed_count, len(files_to_test), tests_failed_count)

    popdir()




################################################################################
#                                opam setup code                               #
################################################################################

WANTED_OPAM_VER = "2.0.0"
WANTED_LLVM_VER = "7.0.0"
OPAM_SWITCH = "4.07.1"

OPAM_PACKAGES_WANTED_MINUS_LLVM = ["camlp4"]

BOLD_FORMAT = '\x1b[1;37;40m'
NORMAL_FORMAT = '\x1b[0m'

def opam_exec_command(arg_list):
    return ["opam", "exec", "--set-switch", OPAM_SWITCH, "--"] + arg_list
EXAMPLE_OPAM_EXEC = command_to_string(opam_exec_command(["<COMMAND>"]))

def opam_setup():
    print("""\


    == Welcome to the opam setup ==

    First, we will check your installed versions of opam and llvm

""")

    print("Checking installed opam version: ", end='')
    opam_version = run_silent(["opam", "--version"]).strip()
    if opam_version == None:
        warning("Consider the following command: sudo pacman -S opam")
        fatal_error("Opam not installed, aborting")
    print(opam_version)

    print("Checking installed llvm version: ", end='')
    llvm_version = run_silent(["llvm-config", "--version"]).strip()
    if llvm_version == None:
        warning("Consider the following command: sudo pacman -S llvm")
        fatal_error("LLVM not installed, aborting")
    print(llvm_version)

    if semantic_version_comp(opam_version, WANTED_OPAM_VER) < 0:
        warning_continue(f"Opam version ({opam_version}) is bellow the desired ({WANTED_OPAM_VER})")

    if semantic_version_comp(llvm_version, WANTED_LLVM_VER) < 0:
        warning_continue(f"LLVM version ({llvm_version}) is bellow the desired ({WANTED_LLVM_VER})")

    llvm_opam_package = "llvm."+llvm_version
    print(f"Using LLVM version to choose opam package: {llvm_opam_package}")
    opam_packages_wanted = OPAM_PACKAGES_WANTED_MINUS_LLVM + [llvm_opam_package]

    print(f"""\


    == Opam initialization ==

    Opam needs to be initialized the first time it's used.
    buildScript.py will then install the following opam switch: {BOLD_FORMAT}{OPAM_SWITCH}{NORMAL_FORMAT}
    In this switch, the following packages are installed: {BOLD_FORMAT}{' '.join(opam_packages_wanted)}{NORMAL_FORMAT}
    Compilation will then by default be done through: {BOLD_FORMAT}{EXAMPLE_OPAM_EXEC}{NORMAL_FORMAT}

""")

    print("If you want to easily execute binaries installed in your opam switch, you can add opam to your PATH.")
    bashrc_edit = parseBool(input_or("Allow opam to add the current switch to PATH when your shell starts? (This modifies your current shell's init script) [N/y]:", "n"))

    if run_loud(["opam", "init", "--bare", "--auto-setup" if bashrc_edit else "--no-setup"]) != 0:
        fatal_error("Opam init failed")
    print()

    run_loud(["opam", "switch", "create", OPAM_SWITCH])
    print()

    if run_loud(["opam", "install", "--switch", OPAM_SWITCH] + opam_packages_wanted) != 0:
        fatal_error("Installing packages failed")

    print()

    if bashrc_edit or parseBool(input_or("Do you want to add opam to PATH in your current session? [y/N]: ", "n")):
        if bashrc_edit:
            print("Adding opam to current shell session:")
        run_loud(["eval", "$(opam env)"], shell=True)
        print("INFO: You have to run this command yourself, I don't know how to export PATH")

    print()

    info("Opam setup is done")

# ====== Functions used by opam setup ======

def input_or(query, default="y"):
    inp = input(query)
    if inp == None or inp == "":
        return default
    return inp

def run_loud(command, **dic):
    print("$ " + command_to_string(command))
    with subprocess.Popen(command, **dic) as com:
        com.wait()
        return com.returncode

def run_silent(command):
    with Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as com: #Forwards stderr
        out, err = com.communicate()
        if com.returncode is not 0:
            return None
        return out.decode('utf-8')

def semantic_version_comp(a, b):
    a = a.split('.')
    b = b.split('.')

    while len(a) > 0 and len(b) > 0:
        if a[0] < b[0]:
            return -1
        elif a[0] > b[0]:
            return 1
        a = a[1:]
        b = b[1:]
    return 0

def warning_continue(form, *args):
    warning(form, *args)
    if not parseBool(input_or("Do you want to continue anyways (probably won't work): [y/N]", "n")):
        exit(0)

if __name__ == "__main__":
    main()

