#!/usr/bin/python

from subprocess import call, Popen, PIPE, check_output
from sys import argv

executable = "../Compiler/build/Debug/DafCompiler"
testFile = "TestFile.daf"
flameGraphFolder="/home/havard/Programs/FlameGraph/"
argList = argv[1:]

print(argList)

if len(argList)==0:
    argList.append(executable)
if len(argList)==1:
    argList.append(testFile)

print("Arg list:", argList)
print("")
print("The following tests are avaliable:")
print("1: Valgrind callgraph")
print("2: Valgrind memory-leak-test")
print("3: Flame graph maker")
print("q: Exit")

inp = input()

if inp=="1":
    call(["valgrind", "--tool=callgrind", "--callgrind-out-file=callgrind.out.tmp"]+argList)
    call(["kcachegrind", "callgrind.out.tmp"])
    call(["rm","callgrind.out.tmp"])
    exit()
elif inp=="2":
    call(["valgrind", "--leak-check=full", "--show-leak-kinds=all"]+argList)
    exit()
elif inp=="3":
    if check_output(["cat", "/proc/sys/kernel/perf_event_paranoid"])[0] != ord('0'):
        print("Must set perf_event_paranoid to 0:")
        call(["sudo", "-s", "su", "-c", "echo \"0\">/proc/sys/kernel/perf_event_paranoid"])
    call(["perf", "record", "-F", "997", "-a", "-g", "--"]+argList)
    script=Popen(["perf", "script"], stdout=PIPE)
    outFile = open("out.perf-folded.tmp", "w+")
    folded=Popen([flameGraphFolder+"stackcollapse-perf.pl"], stdin=script.stdout, stdout=outFile)
    script.wait()
    folded.wait()
    outFile.close()
    outSvg = open("perf-kernel.svg", "w+")
    makeSvg = Popen(["perl", flameGraphFolder+"flamegraph.pl", "--width", "6000", "out.perf-folded.tmp"], stdout=outSvg)
    makeSvg.wait()
    outSvg.close()
    call(["inkscape", "perf-kernel.svg"])
    call(["rm", "perf.data", "perf.data.old", "out.perf-folded.tmp", "perf-kernel.svg"])
    exit()
elif inp=="q":
    exit()
print("ERROR: Unrecoqnized command")
