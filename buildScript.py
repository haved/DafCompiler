#!/bin/env python3
from sys import argv

def error(*a, **ka):
    print("buildScript.py: error: ",end="")
    print(*a, **ka)
    exit(1)

class Option:
    def __init__(self, name, desc, callback, *, take_value=None, allow_multiple=False):
        self.name = name
        self.desc = desc
        self.callback = callback

        self.take_value = take_value if take_value != None else name.endswith("=")
        self.allow_multiple = allow_multiple
        self.executed = False

    def tryExecute(self, text):
        if self.take_value:
            if not text.starts_with(self.name):
                return False
            self.execute(text[len(self.name):])
        else:
            if text != self.name:
                return False
            self.execute()

    def execute(self, *a):
        self.callback(*a)

class Command:
    def __init__(self, name, desc, options, callback):
        self.name = name
        self.desc = desc
        self.options = options
        self.callback = callback

    def execute(self, args):
        def tryOption(arg):
            for opt in self.options:
                if opt.tryExecute(arg):
                    return
            error("Unrecognized option for '{}' command: {}".format(self.name, arg))
        for arg in args:
            tryOption(arg)

        self.callback()

    def get_help_message(self):
        desc_lines = self.desc.split('\n')
        text = "    {:<16}{}\n".format(self.name, desc_lines[0])
        for line in desc_lines[1:]:
            text += "{:<20}{}\n".format("", line)
        return text

class ArgParser:
    def __init__(self):
        self.commands = []
        self.commands_map = {}

    def add_command(self, command):
        self.commands.append(command)
        assert command.name not in self.commands_map
        self.commands_map[command.name]=command

    def execute(self, args):
        if len(args) == 0:
            error("No command given, see --help")
        command_name = args[0]
        if command_name in self.commands_map:
            self.commands_map[command_name].execute(args[1:])
        else:
            error("Unrecognized command: ")

    def get_help_message(self):
        text = "Commands:\n"

        for cmd in self.commands:
            text += cmd.get_help_message()
            text += "\n"
        return text

def main():
    arg_parser = ArgParser()

    

    def print_help_message():
        print("Usage: ./buildScript.py <command> [options]")
        print("Set up an environment and build,test or install the dafc binary")
        print()
        print(arg_parser.get_help_message(),end='')
        exit(0)

    arg_parser.add_command(Command("--help", "Print this help message", [], print_help_message))
    arg_parser.execute(argv[1:])

if __name__ == "__main__":
    main()
