#!/usr/bin/bash

echo "/proc/sys/kernel/perf_event_paranoid is:"
cat /proc/sys/kernel/perf_event_paranoid
#-a ommited here:
perf record -F 997 -g -- ~/Development/daf/DafCompiler/DafCompiler_cpp/build/Debug/DafCompiler TestFile.daf
perf script | ~/Programs/FlameGraph/stackcollapse-perf.pl > out.perf-folded
~/Programs/FlameGraph/flamegraph.pl --width 6000 --colors aqua out.perf-folded > perf-kernel.svg
rm perf.data
rm perf.data.old
rm out.perf-folded
