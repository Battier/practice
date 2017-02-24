echo \n=====================================================\n
echo =================Demo GDB command file==================\n
echo =====================================================\n

echo Turn the output pagination off\n
set pagination off

echo Change the name of the current logfile\n
set logging file gdb.log

echo Enable logging.\n
set logging on

set $i = 0

echo Use cli as program to be debugged.\n
file cli

break main
run

while ( $i < 10 )
  step
  # next
  # continue
  print "Step#%d\n", $i
  set $i = $i + 1
end

echo Show the current values of the logging settings.\n
show logging
