#!/usr/bin/bash

echo "/proc/sys/kernel/perf_event_paranoid should be 0, is:"
cat /proc/sys/kernel/perf_event_paranoid
#-a ommited here:
perf record -F 997 -a -g -- ../Compiler/build/Default/DafCompiler TestFile.daf
perf script | ~/Programs/FlameGraph/stackcollapse-perf.pl > out.perf-folded
~/Programs/FlameGraph/flamegraph.pl --width 6000 --colors aqua out.perf-folded > perf-kernel.svg
rm perf.data
rm perf.data.old
rm out.perf-folded
