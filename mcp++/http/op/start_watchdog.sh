cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -15 `cat $pn"_watchdog.pid"` > /dev/null 2>&1
./$pn"_watchdog"
