#!/usr/bin/bash
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ../Compiler/build/Default/DafCompiler TestFile.daf
kcachegrind callgrind.out
rm callgrind.out
