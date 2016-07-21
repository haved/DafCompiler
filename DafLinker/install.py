#!/usr/bin/python

from system import argv

args = argv[1:]
exec_name = argv[0]

if len(args) == 0:
    log(exec_name+": error: No command passed! Use --help for help")
    exit()
    
if arg == "-h" or arg == "--help":
    print(
"""Installer for dafln, the daf linker
Usage: ./install.py <options> <command>"""")

#cp dafln.py /usr/bin/dafln
#mkdir -p /usr/share/daf
#echo "-I bin -L lib" > /usr/share/daf/linker_search.txt
