#!/bin/bash
 
DEBUGFS=`grep debugfs /proc/mounts | awk '{ print $2; }'`
 
echo 0 > $DEBUGFS/tracing/tracing_on
echo $$ > $DEBUGFS/tracing/set_ftrace_pid
echo function > $DEBUGFS/tracing/current_tracer
echo 1 > $DEBUGFS/tracing/tracing_on
echo "=========================================================="
echo "Here is what is recorded in /sys/kernel/debug/tracing/trace when this script is used to invoke $*"
echo "=========================================================="
exec $*

#TODO: Known issue, exec replaces the current program in the current process, 
#without forking a new process, then exit as soon as it finishes.
echo 0 > $DEBUGFS/tracing/tracing_on
