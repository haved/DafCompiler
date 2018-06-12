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


def doMake():
    prev_cwd = os.getcwd()
    os.chdir("OCompiler")
    info("Compiling dafc")
    with subprocess.Popen(["make", "-j3"]) as makeCall:
        makeCall.wait()
        if makeCall.returncode is not 0:
            fatal_error("Make failed with return code {}", makeCall.returncode)
    os.chdir(prev_cwd)

test_filter = "^.+\\.daf$"
binary_from_test_dir = "../OCompiler/dafc_main.native"
forward_compile_stdout = True
def doTests():
    prev_cwd = os.getcwd()
    os.chdir("OCompilerTests")

    filter = re.compile(test_filter)

    files_to_test = [f_name for f_name in os.listdir('.') if filter.match(f_name)]

    for index, f_name in enumerate(files_to_test, 1):
        info("Running test {}/{}: {}", index, len(files_to_test), f_name)
        with subprocess.Popen([binary_from_test_dir, f_name], stdout=None if forward_compile_stdout else subprocess.PIPE) as comp:
            comp.wait(timeout=10)
            if comp.returncode is not 0:
                fatal_error("Test {}/{} failed: return code {}", index, len(files_to_test), comp.returncode)

    os.chdir(prev_cwd)

def main():
    doMake()
    doTests()

if __name__ == "__main__":
    main()

