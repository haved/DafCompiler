#!/bin/env python3
from sys import argv

version = "0.0.1"
switch_name = "dafc_opam_switch/"
switch_packages = "oasis"
default_test_path = "Tests/"
default_test_filter = ".*\\.daf"

def error(*a, **ka):
    print("buildScript.py: error: ", end="")
    print(*a, **ka)
    exit(1)


def doBuild(options, global_options):
    if len(options) != 0:
        error("Unexpected option for build:", options[0])

    print("Starting build")

def doTest(options, global_options):
    pass

def doInstall(options, global_options):
    if len(options) != 0: #TODO Install location
        error("Unexpected option for install:", options[0])

def doUninstall(options, global_options):
    if len(options) != 0:
        error("Unexpected option for uninstall:", options[0])

def doUpgradeSwitch(options, global_options):
    if len(options) != 0:
        error("Unexpected option for switch_upgrade:", options[0])

def doClean(options, global_options):
    if len(options) != 0:
        error("Unexpected option for clean:", options[0])

def doCleanSwitch(options, global_options):
    if len(options) != 0:
        error("Unexpected option for switch_clean:", options[0])

command_map = {
    "build": doBuild,
    "test": doTest,
    "install": doInstall,
    "uninstall": doUninstall,
    "upgrade-switch": doUpgradeSwitch,
    "clean": doClean,
    "clean-switch": doCleanSwitch,
}

possible_global_options = ["--keep-switch"]

def main():
    args = argv[1:]

    global_options = []
    commands = []

    if len(args) == 0:
        printHelp()

    for arg in args:
        if arg.startswith("-"):
            if arg == "--help":
                printHelp()
            elif arg in possible_global_options:
                global_options.append(arg)
            else:
                if len(commands) == 0:
                    error(arg, "is not a global option, and there is no command specified before it")
                else:
                    commands[-1].options.push_back(arg)
        else:
            commands.append({"name":arg, "options":[]})

    if len(commands) == 0:
        print("No commands specified. See --help for info")

    for command in commands:
        name = command["name"]
        if name not in command_map:
            error("Unrecognized command:", name)
        command_map[name](command["options"], global_options)


def printHelp():
    print("""\
buildScript.py version {}
Usage: ./buildScript.py [commands]
Set up an opam environment and build,test and install the daf compiler

Commands:
    build            Build the daf compiler binary
                     This will init opam for this process only,
                     and make an opam switch named {}
                     unless such a switch already exists.
                     In this switch, we install the packages {}
                     Then oasis is used to make the final binary

    test             Run tests of the compiler
    [--path=path]    If a path is specified, that file / folder is tested
                     Default: {}
    [--filter=rgx]   If a filter is specified, only tests with file names
                     matching the specified regex will be tested
                     Default: {}

    install          Install the built binary to /usr/local/bin
                     Requires build to be called first

    uninstall        Remove files installed by install

    upgrade-switch   Update the opam switch,
                     and install any missing dependencies
                     or newer versions of them

    clean            Remove files and folders created during the build process

    clean-switch     Remove the opam switch made for compiling dafc

Options:
    --keep-switch    Try using the currently set opam switch
                     instead making/using our own.
                     If the current switch is missing packages,
                     try doing upgrade_switch with this option enabled.

    --help           Print this message and exit
""".format(version, switch_name, switch_packages, default_test_path, default_test_filter),end="")
    exit(0)


if __name__ == "__main__":
    main()
