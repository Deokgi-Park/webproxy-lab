target=`ps -ef |grep echoserveri |grep -v 'grep' | awk '{print $2}'`
echo "kill the process PID : $target"
kill -9 $target
